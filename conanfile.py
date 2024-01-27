from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain

# BOOST_CONFIGURE_OPTIONS = (
#     "atomic",
#     "chrono",
#     "container",
#     "context",
#     "contract",
#     "coroutine",
#     "date_time",
#     "exception",
#     "fiber",
#     "filesystem",
#     "graph",
#     "graph_parallel",
#     "iostreams",
#     "json",
#     "locale",
#     "log",
#     "math",
#     "mpi",
#     "nowide",
#     "program_options",
#     "python",
#     "random",
#     "regex",
#     "serialization",
#     "stacktrace",
#     "system",
#     "test",
#     "thread",
#     "timer",
#     "type_erasure",
#     "url",
#     "wave"
# )

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        self.requires("cppzmq/4.10.0")
        self.requires("argh/1.3.2")
        self.requires("cppcodec/0.2")

    # def configure(self):
    #     for boost_module in BOOST_CONFIGURE_OPTIONS:
    #         if boost_module not in [
    #                 "system"
    #         ]:
    #             self.options["boost"][f"without_{boost_module}"] = True


    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")

    def generate(self):
        tc = CMakeToolchain(self)
        if self.settings.compiler == "gcc":
            tc.variables["CMAKE_CXX_FLAGS"] = "-Wall -Wextra -Wcast-qual -Wformat=2 -Wno-format-nonliteral -Wlogical-op -Wmissing-include-dirs -Wno-long-long -Wredundant-decls -Wstrict-overflow=5 -Wundef -pedantic -Wshadow -Wfloat-equal"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def layout(self):
        cmake_layout(self)