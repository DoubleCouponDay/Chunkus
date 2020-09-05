pub mod disco
{
    use std::fs::File;
    use std::io::BufReader;
    use std::io::prelude::*;
    
    pub fn get_token() -> Result<String, std::io::Error>
    {
        let mut file = std::fs::File::open("token.discord_token")?;
        let mut buf_reader = BufReader::new(file);
        let mut token = String::new();
        buf_reader.read_to_string(&mut token)?;
        Ok(token)
    }
}