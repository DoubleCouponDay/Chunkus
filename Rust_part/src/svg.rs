use usvg::SystemFontDB;

pub fn render_svg_to_png(input: &String, output: &String) -> Result<(), String>
{
    
    let mut options = usvg::Options::default();
    options.resources_dir = std::fs::canonicalize(&input).ok().and_then(|p| p.parent().map(|p| p.to_path_buf()));
    options.fontdb.load_system_fonts();
    options.fontdb.set_generic_families();

    let tree: usvg::Tree;
    match usvg::Tree::from_file(&input, &options)
    {
        Ok(rtree) => tree = rtree,
        Err(why) => return Err(format!("{}", why))
    }
    let pixmap_size = tree.svg_node().size.to_screen_size();
    let mut pixmap = tiny_skia::Pixmap::new(pixmap_size.width(), pixmap_size.height()).unwrap();
    if resvg::render(&tree, usvg::FitTo::Original, pixmap.as_mut()).is_none()
    {
        return Err(String::from("Error Rendering SVG"));
    }
    if let Err(why) = pixmap.save_png(&output)
    {
        return Err(format!("{}", why));
    }
    return Ok(());
}