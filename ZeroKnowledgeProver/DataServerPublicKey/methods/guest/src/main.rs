#![no_main]

use risc0_zkvm::guest::env;
use risc0_zkp_core::fp::Fp;

risc0_zkvm::guest::entry!(main);

pub fn main() {

    let (a, e_i, s_i, e_small, s_small): (Vec<Vec<u32>>, Vec<Vec<u32>>, Vec<Vec<u32>>, u32, u32)  = env::read();

    
    // proof that e_i, s_i are small vectors
    e_i.iter().for_each(|e| e.iter().for_each(|a| assert!(*a <= e_small)));
    s_i.iter().for_each(|e| e.iter().for_each(|a| assert!(*a <= s_small)));

    // Convert to vectors and matrix to Fp
    let a = convert_to_fp_matrix(a);
    let e_i = convert_to_fp_matrix(e_i);
    let s_i = convert_to_fp_matrix(s_i);
    

    // Multiply s_i with a 
    let mut res = vec![vec![Fp::default(); a[0].len()]; s_i.len()];
    for i in 0..s_i.len() {
        for j in 0..a[0].len() {
            for k in 0..s_i[0].len() {
                res[i][j] += s_i[i][k] * a[k][j];
            }
        }
    }

    // Add e_i to res
    let mut result: Vec<Vec<Fp>> = vec![vec![Fp::default(); res[0].len()]; res.len()];
    for i in 0..res.len() {
        for j in 0..res[0].len() {
            result[i][j] = res[i][j] + e_i[i][j];
        }
    }

    let result: Vec<Vec<u32>> = convert_to_u32_matrix(result);
    
    // Write the public key to the output
    env::commit(&result);
}

fn convert_to_fp_matrix(matrix: Vec<Vec<u32>>) -> Vec<Vec<Fp>> {
    matrix
        .into_iter()
        .map(|row| row.into_iter().map(Fp::from).collect())
        .collect()
}

fn convert_to_u32_matrix(matrix: Vec<Vec<Fp>>) -> Vec<Vec<u32>> {
    let mut result: Vec<Vec<u32>> = Vec::new();
    for row in matrix {
        let mut u32_row: Vec<u32> = Vec::new();
        for element in row {
            u32_row.push(element.into());
        }
        result.push(u32_row);
    }
    result
}