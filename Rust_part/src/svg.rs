use usvg::SystemFontDB;

pub fn render_svg_to_png(input: &String, output: &String)
{
    
    let mut opt = usvg::Options::default();
    opt.resources_dir = std::fs::canonicalize(&input).ok().and_then(|p| p.parent().map(|p| p.to_path_buf()));
    opt.fontdb.load_system_fonts();
    opt.fontdb.set_generic_families();

    if let Ok(rtree) = usvg::Tree::from_file(&input, &opt)
    {
        let pixmap_size = rtree.svg_node().size.to_screen_size();
        let mut pixmap = tiny_skia::Pixmap::new(pixmap_size.width(), pixmap_size.height()).unwrap();
        resvg::render(&rtree, usvg::FitTo::Original, pixmap.as_mut()).unwrap();
        pixmap.save_png(&output).unwrap();
    }
}