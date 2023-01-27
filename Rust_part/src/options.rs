use core::clone::Clone;

use serenity::{
    prelude::{
        TypeMapKey,
        TypeMap,
    },
    client::Context
};
use tokio::sync::RwLockWriteGuard;

pub const DEFAULT_CHUNK_SIZE: u32 = 1;
pub const DEFAULT_THRESHOLDS: u32 = 3;
pub const DEFAULT_COLOURS: u32 = 256;

pub struct VectorizeOptionsKey;

pub struct VectorizeOptions
{
    pub chunksize: u32,
    pub thresholds: u32,
    pub numcolours: u32,
    pub shouldcrash: bool
}

pub struct ParsedOptions { //we need strings in order to make the char** for the core
    pub chunksize: String,
    pub thresholds: String,
    pub numcolours: String,
    pub shouldcrash: String
}

impl TypeMapKey for VectorizeOptionsKey
{
    type Value = ParsedOptions;
}

impl Clone for ParsedOptions {
    fn clone(&self) -> ParsedOptions {
        ParsedOptions {
            chunksize: self.chunksize.clone(),
            thresholds: self.thresholds.clone(),
            numcolours: self.numcolours.clone(),
            shouldcrash: self.shouldcrash.clone()
        }
    }
}

fn choose_u32(input: u32, constant: u32) -> String {
    match input {
        0 => constant.to_string(),
        _ => input.to_string()
    }
}

pub fn insert_params(mut data: RwLockWriteGuard<'_, TypeMap>, input: VectorizeOptions) -> ParsedOptions {
    println!("inserting params. shouldcrash: {}", input.shouldcrash);

    let options = ParsedOptions {
        chunksize: choose_u32(input.chunksize, DEFAULT_CHUNK_SIZE),
        thresholds: choose_u32(input.thresholds, DEFAULT_THRESHOLDS),
        numcolours: choose_u32(input.numcolours, DEFAULT_COLOURS),
        shouldcrash: input.shouldcrash.to_string()
    };
    data.insert::<VectorizeOptionsKey>(options.clone());
    options
}

pub fn insert_parsed_params(mut data: RwLockWriteGuard<'_, TypeMap>, input: ParsedOptions) {
    println!("inserting params. shouldcrash: {}", input.shouldcrash);
    data.insert::<VectorizeOptionsKey>(input.clone());
    ()
}

pub async fn get_params(ctx: &Context) -> ParsedOptions {
    let data_read = ctx.data.write().await;
    let borrowed_options = data_read.get::<VectorizeOptionsKey>().unwrap();
    borrowed_options.clone()
}
