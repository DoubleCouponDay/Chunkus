mod secrettoken;
mod bot;
mod core;

#[tokio::main]
async fn main() {
    //start discord bot
    let token = secrettoken::gettoken();
    let mut client = bot::create_bot(&token).await;
    let _ = client.start();
}
