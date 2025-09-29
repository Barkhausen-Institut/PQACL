use risc0_zkvm::guest::env;
use risc0_zkp_core::fp::Fp;

fn main() {
    let (s, c, d): (Vec<Vec<u32>>, Vec<u32>, Vec<u32>) = env::read();

    // Convert to vectors and matrix to Fp
    let s = convert_to_fp_matrix(s);
    let c = convert_to_fp_vector(c);
    let d = convert_to_fp_vector(d);

    // Calculate x
    // Multiply s with c
    let mut res1 = vec![Fp::default(); s.len()];
    for i in 0..s.len() {
        for j in 0..s[0].len(){
            res1[i] += s[i][j] * c[j];
        }
    }

    // Subtract res from d
    let mut x = vec![Fp::default(); res1.len()];
    for i in 0..res1.len() {
        x[i] = d[i] - res1[i];
    }

    // Convert the results back to u32 vectors
    let x: Vec<u32> = convert_to_u32_vector(x);

    // Write results to the output
    env::commit(&x);
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
