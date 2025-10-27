from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps

class MyProjectConan(ConanFile):
    name = "myproject"
    version = "0.1"

    settings = "os", "arch", "compiler", "build_type"
    requires = (
        "drogon/1.9.7",
        "gtest/1.16.0",
        "jwt-cpp/0.4.0"
    )
    exports_sources = "CMakeLists.txt", "src/*", "tests/*"

    options = {
        "asan": [True, False],
        "ubsan": [True, False],
        "extra_warnings": [True, False],
        "extra_link_time_opts": [True, False],
        "run_tests": [True, False],
    }
    default_options = {
        "drogon/*:with_postgres": True,
        "drogon/*:with_mysql": False,
        "drogon/*:with_sqlite3": False,


        "asan": False,
        "ubsan": False,
        "extra_warnings": True,
        "extra_link_time_opts": False,
        "run_tests": False,
    }

    def _compiler_str(self):
        return str(self.settings.compiler) if self.settings.get_safe("compiler") else ""

    def _is_msvc(self):
        comp = self._compiler_str().lower()
        return "msvc" in comp or "visual studio" in comp

    def _is_clang(self):
        comp = self._compiler_str().lower()
        # apple-clang reported sometimes as 'apple-clang'
        return "clang" in comp or "apple-clang" in comp

    def _is_gcc(self):
        comp = self._compiler_str().lower()
        # Note: mingw often reports compiler as gcc on Windows; treat it separately when needed
        return "gcc" in comp and "mingw" not in comp

    def _is_mingw(self):
        # Heuristic: OS windows + gcc => mingw (not foolproof)
        is_win = str(self.settings.os).lower() == "windows" if self.settings.get_safe("os") else False
        return is_win and "gcc" in self._compiler_str().lower()

    def generate(self):
        tc = CMakeToolchain(self)

        # basic preprocessor definitions
        tc.preprocessor_definitions["PROJECT_VERSION"] = f'"{self.version}"'
        tc.preprocessor_definitions.debug["BUILD_DEBUG"] = "1"
        tc.preprocessor_definitions.release["NDEBUG"] = "1"

        # lists of extra flags
        extra_cxx = []
        extra_c = []
        extra_exe_link = []
        extra_shared_link = []

        build_type = self.settings.get_safe("build_type")
        if build_type and str(build_type).lower() == "debug":
            extra_cxx.append("-g")
            extra_c.append("-g")
            extra_cxx.append("-fno-omit-frame-pointer")
            extra_c.append("-fno-omit-frame-pointer")

        if bool(self.options.extra_warnings):
            if self._is_msvc():
                extra_cxx += ["/W4"]
            else:
                extra_cxx += ["-Wall", "-Wextra", "-Wpedantic"]

        # sanitizers handling
        if bool(self.options.asan) or bool(self.options.ubsan):
            sanitizers = []
            if bool(self.options.asan):
                sanitizers.append("address")
            if bool(self.options.ubsan):
                sanitizers.append("undefined")

            # GCC / Clang / MinGW: use -fsanitize
            if self._is_gcc() or self._is_clang() or self._is_mingw():
                san_flag = "-fsanitize=" + ",".join(sanitizers)
                extra_cxx.append(san_flag)
                extra_c.append(san_flag)
                extra_exe_link.append(san_flag)
                extra_shared_link.append(san_flag)

                if bool(self.options.ubsan):
                    extra_cxx.append("-fno-sanitize-recover=undefined")
                    extra_c.append("-fno-sanitize-recover=undefined")

                # ensure debug info & frame pointers for sanitizer backtraces
                extra_cxx.append("-fno-omit-frame-pointer")
                extra_c.append("-fno-omit-frame-pointer")
                extra_cxx.append("-g")
                extra_c.append("-g")

            elif self._is_msvc():
                # MSVC sanitizer support is limited and depends on Visual Studio version.
                # For MSVC/clang-cl newer VS versions can accept /fsanitize=address (or use clang-cl).
                # We add a conservative MSVC flag attempt, but it may need adjustment per VS version.
                extra_cxx.append("/fsanitize=address")
                extra_exe_link.append("/fsanitize=address")
                # Note: if unsupported, the build will fail — adjust by toolchain/profile accordingly.

            else:
                # Unknown compiler: provide a preprocessor define to let CMakeLists handle it.
                tc.preprocessor_definitions["SANITIZERS_ENABLED"] = "0"

            if bool(self.options.asan):
                tc.preprocessor_definitions["USE_ASAN"] = "1"
            if bool(self.options.ubsan):
                tc.preprocessor_definitions["USE_UBSAN"] = "1"

        # extra linker options
        if bool(self.options.extra_link_time_opts):
            if self._is_msvc():
                extra_exe_link.append("/INCREMENTAL:NO")
            else:
                extra_exe_link.append("-Wl,--as-needed")

        # Safely prepend/append flags to toolchain attributes (they may be None initially)
        # getattr with default empty list ensures we don't crash.
        tc.extra_cxxflags = extra_cxx + list(getattr(tc, "extra_cxxflags", []) or [])
        tc.extra_cflags = extra_c + list(getattr(tc, "extra_cflags", []) or [])
        tc.extra_exelinkflags = extra_exe_link + list(getattr(tc, "extra_exelinkflags", []) or [])
        tc.extra_sharedlinkflags = extra_shared_link + list(getattr(tc, "extra_sharedlinkflags", []) or [])

        # example config-dependent variables/flags
        tc.variables.debug["MY_DEBUG_VAR"] = "ON"
        tc.variables.release["MY_RELEASE_VAR"] = "ON"

        # If run_tests requested, instruct CMake to enable BUILD_TESTING so tests are built
        if bool(self.options.run_tests):
            tc.variables["BUILD_TESTING"] = "ON"

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if bool(self.options.run_tests):
            try:
                cmake.ctest(cli_args=["--output-on-failure"])
            except Exception:
                cmake.test()