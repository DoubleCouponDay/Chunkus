
-- add modes: debug and release
add_rules("mode.debug")
add_rules("mode.release")
add_requires("CONAN::libpng/1.6.37", {alias = "libpng"})
add_requires("CONAN::libjpeg/9d", {alias = "libjpeg"})
add_requires("CONAN::nanosvg/20190405", {alias = "nanosvg"})

if is_mode("debug") then
    add_defines("DEBUG")
end

rule("copytemplate")
    after_build(function (target)
        print("Target Dir: %s", target:targetdir())
        os.cp("../template.svg", path.join(target:targetdir(), "template.svg"))
    end)

target("tests")
    if is_plat("linux") then
        add_syslinks("m")
    end
    -- set rule
    add_rules("copytemplate")
    -- set kind
    set_kind("binary")
    -- add files
    add_files("./src/**.c|main.c")
    add_files("./test/**.c")
    add_includedirs("./src/")
    add_packages("libpng", "libjpeg", "nanosvg", "sort")

target("vec")
    if is_plat("linux") then
        add_syslinks("m")
    end
    set_kind("static")
    add_files("./src/**.c|main.c")
    add_includedirs("./src/")
    add_packages("libpng", "libjpeg", "nanosvg", "sort")
