#![allow(non_snake_case)]

extern crate bulletproofs;
extern crate curve25519_dalek;
extern crate merlin;
extern crate rand;

use bulletproofs::r1cs::*;
use bulletproofs::{BulletproofGens, PedersenGens};
use curve25519_dalek::scalar::Scalar;
use merlin::Transcript;
use std::{error::Error, fs, path::PathBuf, env, time::Instant};

fn main() -> Result<(), Box<dyn Error>>{
    // use executable as: ./prover m l k data_path proof_path
    let args: Vec<String> = env::args().collect();
    let m : usize = args[1].parse().unwrap();
    let l : usize = args[2].parse().unwrap();
    let k : usize = args[3].parse().unwrap();
    let data_path = &args[4];
    let _proof_path = &args[5];

    // read the JSON data to vector
    let data = fs::read_to_string(PathBuf::from(data_path))?;
    let data = json::parse(&data).unwrap();

    // init the vectors
    let mut s : Vec<Vec<u32>> = Vec::new();
    let mut c : Vec<u32> = Vec::new();
    let mut d : Vec<u32> = Vec::new();
    let mut x : Vec<u32> = Vec::new();

    // load the data from JSON
    for i in 0..k{
        c.push(data["c_1"][i].as_u32().unwrap());
    }
    for i in 0..m*l{
        d.push(data["d_i"][i].as_u32().unwrap());
        x.push(data["x_i"][i].as_u32().unwrap());
        let mut row_s : Vec<u32> = Vec::new();
        for j in 0..k{
            row_s.push(data["s_i"][i][j].as_u32().unwrap());
        }
        s.push(row_s);
    }

    let now = Instant::now();
    // prove
    let pc_gens = PedersenGens::default();
    let bp_gens = BulletproofGens::new(32768, 1);
    let mut prover_transcript = Transcript::new(b"ZKP_Dec");
    let mut prover = Prover::new(&pc_gens, &mut prover_transcript);
    let mut rng = rand::thread_rng();

    let (c_cs, c_vs): (Vec<_>, Vec<_>) = c
        .into_iter()
        .map(|x| prover.commit(Scalar::from(x), Scalar::random(&mut rng)))
        .unzip();
    let (d_cs, d_vs): (Vec<_>, Vec<_>) = d
        .into_iter()
        .map(|x| prover.commit(Scalar::from(x), Scalar::random(&mut rng)))
        .unzip();
    let mut s_cs = Vec::new();
    let mut s_vs = Vec::new();
    for i in 0..m*l {
        let (row_cs, row_vs): (Vec<_>, Vec<_>) = s[i].clone()
            .into_iter()
            .map(|x| prover.commit(Scalar::from(x), Scalar::random(&mut rng)))
            .unzip();
        s_cs.push(row_cs);
        s_vs.push(row_vs);
        let mut sc = LinearCombination::from(0u32);
        for j in 0..k {
            let (_, _, sc_var) = prover.multiply(s_vs[i][j].into(), c_vs[j].into());
            sc = sc + sc_var;
        }
        prover.constrain(sc + LinearCombination::from(x[i]) - d_vs[i]);
    }

    let proof = prover.prove(&bp_gens)?;
    let elapsed = now.elapsed();
    println!("ZKP Bulletproofs Dec Prove: {:.2?} ms", elapsed.as_millis());

    // let proof = proof.to_bytes();
    // let proof = R1CSProof::from_bytes(&proof)?;

    let now = Instant::now();
    // verify
    let mut verifier_transcript = Transcript::new(b"ZKP_Dec");
    let mut verifier = Verifier::new(&mut verifier_transcript);
    let mut var_c = Vec::new();
    for i in 0..k{
        var_c.push(verifier.commit(c_cs[i]));
    }
    let mut var_d = Vec::new();
    let mut var_s = Vec::new();
    for i in 0..m*l{
        var_d.push(verifier.commit(d_cs[i]));
        let mut row_s = Vec::new();
        for j in 0..k{
            row_s.push(verifier.commit(s_cs[i][j]));
        }
        var_s.push(row_s);
    }
    for i in 0..m*l {
        let mut sc = LinearCombination::from(0u32);
        for j in 0..k {
            let (_, _, sc_var) = verifier.multiply(var_s[i][j].into(), var_c[j].into());
            sc = sc + sc_var;
        }
        verifier.constrain(sc + LinearCombination::from(x[i]) - var_d[i]);
    }

    let _ = verifier.verify(&proof, &pc_gens, &bp_gens).map_err(|_| R1CSError::VerificationError);
    let elapsed = now.elapsed();
    println!("ZKP Bulletproofs Dec Verify: {:.2?} ms", elapsed.as_millis());

    let bsize = proof.serialized_size() + (k+m*l+m*l*k)*32;
    println!("ZKP Bulletproofs Dec Size: {} bytes", bsize);
    Ok(())
}