from conans import ConanFile
from conans import CMake


class Cis1WebUINativeSrvCpp(ConanFile):
    name = "cis1-webui-native-srv-cpp"
    version = "0.0.1"
    description = "CIS1 webui server implementation."
    author = "MokinIA <mia@tomsksoft.com>"
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    exports = []
    exports_sources = [
        "CMakeLists.txt",
        "include/*",
        "src/*",
        "example_config.ini",
        "cmake/*"]
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
                "rapidjson/1.1.0@bincrafters/stable",
                "openssl/1.1.1c@_/_",
                "sqlite_orm/1.3@tomsksoft/cis1",
                "cis1_tpl/0.0.3@tomsksoft/cis1",
                "cis1_cwu_transport/0.0.5@tomsksoft/cis1",
                "cis1_cwu_protocol/0.0.5@tomsksoft/cis1",
                "cis1_proto_utils/0.0.8@tomsksoft/cis1",
                "sc_logger/1.0.3@tomsksoft/cis1",
                "croncpp/1.0@tomsksoft/cis1")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.cmake", dst="cmake/modules", src="cmake/modules")
        self.copy("cis1-srv", dst="bin", src="bin")
        self.copy("libcis1_srv_impl.a", dst="lib", src="lib")
        self.copy("libcis1_srv_impl.lib", dst="lib", src="lib")

    def package_info(self):
        self.cpp_info.builddirs = ["", "cmake/modules"]
        self.cpp_info.libs = ["cis1_srv_impl"]
