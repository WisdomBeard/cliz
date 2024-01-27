from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        self.requires("cppzmq/4.10.0")
        self.requires("argh/1.3.2")
        self.requires("cppcodec/0.2")

    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def layout(self):
        cmake_layout(self)