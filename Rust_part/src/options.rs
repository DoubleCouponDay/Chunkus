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
const DEFAULT_THRESHOLD: f32 = 1.0;
const DEFAULT_COLOURS: u32 = 256;

pub struct VectorizeOptionsKey;

pub struct VectorizeOptions
{
    pub chunksize: u32,
    pub thresholds: f32,
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

fn place_default_u32(input: u32, constant: u32) -> String {
    match input {
        0 => constant.to_string(),
        _ => input.to_string()
    }
}

fn place_default_f32(input: f32, constant: f32) -> String {
    if input == 0.0 {
        constant.to_string()
    } else {
        input.to_string()
    }
}

pub async fn insert_params(mut data: RwLockWriteGuard<'_, TypeMap>, input: VectorizeOptions) -> ParsedOptions {
    println!("inserting params. shouldcrash: {}", input.shouldcrash);

    let options = ParsedOptions {
        chunksize: place_default_u32(input.chunksize, DEFAULT_CHUNK_SIZE),
        thresholds: place_default_f32(input.thresholds, DEFAULT_THRESHOLD),
        numcolours: place_default_u32(input.numcolours, DEFAULT_COLOURS),
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
