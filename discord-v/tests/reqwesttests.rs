#[cfg(test)]
mod tests {
    use reqwest::Client;
    use std::fs::File;
    use std::env;
    use std::io::prelude::*;
    use std::path::Path;    

    #[tokio::test]
    async fn test_reqwest() -> Result<(), std::io::Error> {
        let client = reqwest::Client::new();
        let response = match client.get("https://cdn.vox-cdn.com/thumbor/7BMHi-0CALa3odBFMD-MJK9Ye4Y=/0x44:1268x889/1200x800/filters:focal(0x44:1268x889)/cdn.vox-cdn.com/uploads/chorus_image/image/47684009/Screenshot_2014-07-19_15.24.57.0.png")
            .send().await
            {
                Err(_) => panic!("wtf"),
                Ok(thing) => thing
            };
        
        let mut file = File::create(Path::new("gotem lmao xddd.bmp"))?;

        let cwd = env::current_dir()?;
        println!("Cwd: {}", cwd.display());

        match response.bytes().await
        {
            Ok(bytes) => file.write_all(&bytes)?,
            Err(_) => panic!("wtf"),
        };
        file.sync_all()?;
        Ok(())
    }
}
