from conans import ConanFile
from conans import CMake


class Cis1CoreNative(ConanFile):
    name = "cis1-webui-native-srv-cpp"
    version = "0.0.0"
    settings = "os", "arch", "compiler", "build_type"
    generators = "cmake"
    exports = "*"
    requires = ("gtest/1.8.1@bincrafters/stable",
                "boost_beast/1.69.0@bincrafters/stable",
                "boost_process/1.69.0@bincrafters/stable",
                "boost_filesystem/1.69.0@bincrafters/stable",
                "boost_system/1.69.0@bincrafters/stable",
                "boost_asio/1.69.0@bincrafters/stable",
                "boost_intrusive/1.69.0@bincrafters/stable",
                "boost_lexical_cast/1.69.0@bincrafters/stable",
                "boost_property_tree/1.69.0@bincrafters/stable",
                "boost_optional/1.69.0@bincrafters/stable",
                "boost_assert/1.69.0@bincrafters/stable",
                "sqlite_orm/1.3@demo/testing",
                "rapidjson/1.1.0@bincrafters/stable",
                "OpenSSL/1.1.1a@conan/stable",
                "ccwu_transport/0.0.0@demo/testing",
                "ccwu_protocol/0.0.0@demo/testing",
                "cis1_proto_utils/0.0.0@demo/testing")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("cis1-srv", dst="bin", src="bin")
        self.copy("libcis1_srv_lib.a", dst="lib", src="lib")
        self.copy("libcis1_srv_lib.lib", dst="lib", src="lib")
        self.copy("FindFilesystem.cmake", dst="cmake/modules", src="cmake/modules")

    def package_info(self):
        self.cpp_info.builddirs = ["", "cmake/modules"]
        self.cpp_info.libs = ["cis1_srv_lib"]
