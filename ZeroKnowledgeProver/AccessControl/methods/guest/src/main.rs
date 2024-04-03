#![no_main]

use risc0_zkvm::guest::env;
use risc0_zkp_core::fp::Fp;

risc0_zkvm::guest::entry!(main);

pub fn main() {

    let (a, b, e_1, e_2, m, r, e_small, s_small): (Vec<Vec<u32>>, Vec<Vec<u32>>, Vec<u32>, Vec<u32>, Vec<u32>, Vec<u32>, u32, u32)   = env::read();

    // proof that e_1, e_2 and r are small vectors
    e_1.iter().for_each(|e| assert!(*e <= e_small));
    e_2.iter().for_each(|e| assert!(*e <= e_small));
    r.iter().for_each(|e| assert!(*e <= s_small));

    // Convert to vectors and matrix to Fp
    let a = convert_to_fp_matrix(a);
    let b = convert_to_fp_matrix(b);
    let e_1 = convert_to_fp_vector(e_1);
    let e_2 = convert_to_fp_vector(e_2);
    let m = convert_to_fp_vector(m);
    let r = convert_to_fp_vector(r);



    // Calculate c_1
    // Multiply a with r 
    let mut res1 = vec![Fp::default(); a.len()];
    for i in 0..a.len() {
        for j in 0..a[0].len(){
            res1[i] += a[i][j] * r[j];
        }
    }

    // Add e_1 to res
    let mut c_1 = vec![Fp::default(); res1.len()];
    for i in 0..res1.len() {
        c_1[i] += res1[i] + e_1[i];
    }

    // Calculate C_2
    // Multiply b with r
    let mut res2 = vec![Fp::default(); b.len()];
    for i in 0..b.len() {
        for j in 0..b[0].len(){
            res2[i] += b[i][j] * r[j];
        }
    }

    // Add e_2 and m to res
    let mut c_2 = vec![Fp::default(); res2.len()];
    for i in 0..res2.len() {
        c_2[i] += res2[i] + e_2[i] + m[i];
    }


    let c_1: Vec<u32> = convert_to_u32_vector(c_1);
    let c_2: Vec<u32> = convert_to_u32_vector(c_2);

    // Write the public key to the output
    env::commit(&c_1);
    env::commit(&c_2);
}

fn convert_to_fp_matrix(matrix: Vec<Vec<u32>>) -> Vec<Vec<Fp>> {
    matrix
        .into_iter()
        .map(|row| row.into_iter().map(Fp::from).collect())
        .collect()
}
fn convert_to_fp_vector(vector: Vec<u32>) -> Vec<Fp> {
    vector.into_iter().map(Fp::from).collect()
}

fn convert_to_u32_vector(vector: Vec<Fp>) -> Vec<u32> {
    let mut u32_row: Vec<u32> = Vec::new();
    for element in vector {
        u32_row.push(element.into());
    }
    u32_row
}
