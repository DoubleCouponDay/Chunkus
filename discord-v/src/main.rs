pub mod secrettoken;
pub mod bot;

#[tokio::main]
async fn main() {
    //start discord bot
    let token = secrettoken::gettoken();
    //let bot::create_bot(&token);
    //register event hooks

    //take any message from discord server
    //invoke the core
    //send a message to the discord server
    let http = Http::new_with_token(&token);

}
