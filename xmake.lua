
-- add modes: debug and release
add_rules("mode.debug")
add_defines("DEBUG")
add_requires("CONAN::libpng/1.6.37", {alias = "libpng"})
add_requires("CONAN::libjpeg/9d", {alias = "libjpeg"})
add_requires("CONAN::nanosvg/20190405", {alias = "nanosvg"})

-- 
target("vectorizer_standalone")
    if is_plat("linux") then
        add_syslinks("m")
    end
    -- set kind
    set_kind("binary")
    -- add files
    add_files("./src/**.c")
    add_packages("libpng", "libjpeg", "nanosvg")

target("tests")
    if is_plat("linux") then
        add_syslinks("m")
    end
    -- set kind
    set_kind("binary")
    -- add files
    add_files("./src/**.c|main.c")
    add_files("./test/tests.c", "./test/munit.c", "./test/**.c")
    add_packages("libpng", "libjpeg", "nanosvg")

target("staticvectorizer")
    if is_plat("linux") then
        add_syslinks("m")
    end
    set_kind("static")
    add_files("./src/**.c|main.c")
    add_packages("libpng", "libjpeg", "nanosvg")
