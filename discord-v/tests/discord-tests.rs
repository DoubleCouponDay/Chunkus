#[cfg(test)]
mod tests {

    #[path = "../../src/discord_stuff.rs"]
    mod d;

    use std::result::Result;

    #[test]
    fn token_obtainable() -> Result<(), std::io::Error>  {
        
        d::disco::get_token()?;
        Ok(())
    }


}