use core::clone::Clone;

use serenity::{
    prelude::{
        TypeMapKey,
        TypeMap,
    },
    client::Context
};
use tokio::sync::RwLockWriteGuard;

const DEFAULT_CHUNK_SIZE: u32 = 1;
const DEFAULT_THRESHOLD: u32 = 1;
const DEFAULT_COLOURS: u32 = 256;

pub struct VectorizeOptionsKey;

pub struct VectorizeOptions
{
    pub chunksize: u32,
    pub threshold: u32,
    pub numcolours: u32,
    pub shouldcrash: bool
}

pub struct ParsedOptions { //we need strings in order to make the char** for the core
    pub chunksize: String,
    pub threshold: String,
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
            threshold: self.threshold.clone(),
            numcolours: self.numcolours.clone(),
            shouldcrash: self.shouldcrash.clone()
        }
    }
}

fn place_default_if_needed(input: u32, constant: u32) -> String {
    match input {
        0 => constant.to_string(),
        _ => input.to_string()
    }
}

pub async fn insert_params(mut data: RwLockWriteGuard<'_, TypeMap>, input: VectorizeOptions) -> ParsedOptions {

    let options = ParsedOptions {
        chunksize: place_default_if_needed(input.chunksize, DEFAULT_CHUNK_SIZE),
        threshold: place_default_if_needed(input.threshold, DEFAULT_THRESHOLD),
        numcolours: place_default_if_needed(input.numcolours, DEFAULT_COLOURS),
        shouldcrash: input.shouldcrash.to_string()
    };
    data.insert::<VectorizeOptionsKey>(options.clone());
    options
}

pub async fn get_params(ctx: &Context) -> ParsedOptions {
    let data_read = ctx.data.write().await;
    let borrowed_options = data_read.get::<VectorizeOptionsKey>().unwrap();
    borrowed_options.clone()
}
