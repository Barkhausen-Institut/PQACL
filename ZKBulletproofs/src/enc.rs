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
    let mut a : Vec<Vec<u32>> = Vec::new();
    let mut b : Vec<Vec<u32>> = Vec::new();
    let mut e1 : Vec<u32> = Vec::new();
    let mut e2 : Vec<u32> = Vec::new();
    let mut r : Vec<u32> = Vec::new();
    let mut x : Vec<u32> = Vec::new();
    let mut c1 : Vec<u32> = Vec::new();
    let mut c2 : Vec<u32> = Vec::new();

    // load the data from JSON
    for i in 0..k{
        r.push(data["r"][i].as_u32().unwrap());
        e1.push(data["e_1"][i].as_u32().unwrap());
        let mut row_a : Vec<u32> = Vec::new();
        for j in 0..k{
            row_a.push(data["A"][i][j].as_u32().unwrap());
        }
        a.push(row_a);
        c1.push(data["c_1"][i].as_u32().unwrap());
    }
    for i in 0..m*l{
        x.push(data["priv_key"][i].as_u32().unwrap());
        e2.push(data["e_2"][i].as_u32().unwrap());
        c2.push(data["c_2"][i].as_u32().unwrap());
        let mut row_b : Vec<u32> = Vec::new();
        for j in 0..k{
            row_b.push(data["B"][i][j].as_u32().unwrap());
        }
        b.push(row_b);
    }

    let now = Instant::now();
    // prove
    let pc_gens = PedersenGens::default();
    let bp_gens = BulletproofGens::new(32768, 1);
    let mut prover_transcript = Transcript::new(b"ZKP_Enc");
    let mut prover = Prover::new(&pc_gens, &mut prover_transcript);
    let mut rng = rand::thread_rng();

    let (mut r_c_p, mut r_v_p) = (Vec::new(), Vec::new());
    let (mut e1_c_p, mut e1_v_p) = (Vec::new(), Vec::new());
    let (mut a_c_p, mut a_v_p) = (Vec::new(), Vec::new());
    for i in 0..k {
        let (c, v) = prover.commit(Scalar::from(r[i]), Scalar::random(&mut rng));
        r_c_p.push(c);
        r_v_p.push(v);
        let (c, v) = prover.commit(Scalar::from(e1[i]), Scalar::random(&mut rng));
        e1_c_p.push(c);
        e1_v_p.push(v);
        let (mut row_c, mut row_v) = (Vec::new(), Vec::new());
        for j in 0..k {
            let (c, v) = prover.commit(Scalar::from(a[i][j]), Scalar::random(&mut rng));
            row_c.push(c);
            row_v.push(v);
        }
        a_c_p.push(row_c);
        a_v_p.push(row_v);
    }
    let (mut x_c_p, mut x_v_p) = (Vec::new(), Vec::new());
    let (mut e2_c_p, mut e2_v_p) = (Vec::new(), Vec::new());
    let (mut b_c_p, mut b_v_p) = (Vec::new(), Vec::new());
    for i in 0..m*l {
        let (c, v) = prover.commit(Scalar::from(x[i]), Scalar::random(&mut rng));
        x_c_p.push(c);
        x_v_p.push(v);
        let (c, v) = prover.commit(Scalar::from(e2[i]), Scalar::random(&mut rng));
        e2_c_p.push(c);
        e2_v_p.push(v);
        let (mut row_c, mut row_v) = (Vec::new(), Vec::new());
        for j in 0..k {
            let (c, v) = prover.commit(Scalar::from(b[i][j]), Scalar::random(&mut rng));
            row_c.push(c);
            row_v.push(v);
        }
        b_c_p.push(row_c);
        b_v_p.push(row_v);
    }
    for i in 0..k {
        let mut ar = LinearCombination::from(0u32);
        for j in 0..k {
            let (_, _, ar_v) = prover.multiply(a_v_p[i][j].into(), r_v_p[j].into());
            ar = ar + ar_v;
        }
        prover.constrain(ar + e1_v_p[i] - LinearCombination::from(c1[i]));
    }
    for i in 0..m*l {
        let mut br = LinearCombination::from(0u32);
        for j in 0..k {
            let (_, _, br_v) = prover.multiply(b_v_p[i][j].into(), r_v_p[j].into());
            br = br + br_v;
        }
        prover.constrain(br + e2_v_p[i] + x_v_p[i] - LinearCombination::from(c2[i]));
    }

    let proof = prover.prove(&bp_gens)?;
    let elapsed = now.elapsed();
    println!("ZKP Bulletproofs Enc Prove: {:.2?} ms", elapsed.as_millis());

    // let proof = proof.to_bytes();
    // let proof = R1CSProof::from_bytes(&proof)?;

    let now = Instant::now();
    // verify
    let mut verifier_transcript = Transcript::new(b"ZKP_Enc");
    let mut verifier = Verifier::new(&mut verifier_transcript);
    let (mut r_v_v, mut e1_v_v, mut a_v_v) = (Vec::new(), Vec::new(), Vec::new());
    for i in 0..k {
        r_v_v.push(verifier.commit(r_c_p[i]));
        e1_v_v.push(verifier.commit(e1_c_p[i]));
        let mut row_v = Vec::new();
        for j in 0..k {
            row_v.push(verifier.commit(a_c_p[i][j]));
        }
        a_v_v.push(row_v);
    }
    let (mut x_v_v, mut e2_v_v, mut b_v_v) = (Vec::new(), Vec::new(), Vec::new());
    for i in 0..m*l {
        x_v_v.push(verifier.commit(x_c_p[i]));
        e2_v_v.push(verifier.commit(e2_c_p[i]));
        let mut row_v = Vec::new();
        for j in 0..k {
            row_v.push(verifier.commit(b_c_p[i][j]));
        }
        b_v_v.push(row_v);
    }
    for i in 0..k {
        let mut ar = LinearCombination::from(0u32);
        for j in 0..k {
            let (_, _, ar_v) = verifier.multiply(a_v_v[i][j].into(), r_v_v[j].into());
            ar = ar + ar_v;
        }
        verifier.constrain(ar + e1_v_v[i] - LinearCombination::from(c1[i]));
    }
    for i in 0..m*l {
        let mut br = LinearCombination::from(0u32);
        for j in 0..k {
            let (_, _, br_v) = verifier.multiply(b_v_v[i][j].into(), r_v_v[j].into());
            br = br + br_v;
        }
        verifier.constrain(br + e2_v_v[i] + x_v_v[i] - LinearCombination::from(c2[i]));
    }

    let _ = verifier.verify(&proof, &pc_gens, &bp_gens).map_err(|_| R1CSError::VerificationError);
    let elapsed = now.elapsed();
    println!("ZKP Bulletproofs Enc Verify: {:.2?} ms", elapsed.as_millis());

    let bsize = proof.serialized_size() + (k+k+k*k+m*l+m*l+m*l*k)*32;
    println!("ZKP Bulletproofs Enc Size: {} bytes", bsize);
    Ok(())
}