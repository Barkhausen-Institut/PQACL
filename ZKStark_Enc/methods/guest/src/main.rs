use risc0_zkvm::guest::env;
use risc0_zkp_core::fp::Fp;

fn main() {
    let (a, b, e1, e2, r, x): (Vec<Vec<u32>>, Vec<Vec<u32>>, Vec<u32>, Vec<u32>, Vec<u32>, Vec<u32>) = env::read();

    // Convert to vectors and matrix to Fp
    let a = convert_to_fp_matrix(a);
    let b = convert_to_fp_matrix(b);
    let e1 = convert_to_fp_vector(e1);
    let e2 = convert_to_fp_vector(e2);
    let r = convert_to_fp_vector(r);
    let x = convert_to_fp_vector(x);

    // Calculate c1
    // Multiply a with r
    let mut res1 = vec![Fp::default(); a.len()];
    for i in 0..a.len() {
        for j in 0..a[0].len(){
            res1[i] += a[i][j] * r[j];
        }
    }

    // Add e1 to res
    let mut c1 = vec![Fp::default(); res1.len()];
    for i in 0..res1.len() {
        c1[i] = res1[i] + e1[i];
    }

    // Calculate c2
    // Multiply B with r
    let mut res2 = vec![Fp::default(); b.len()];
    for i in 0..b.len() {
        for j in 0..b[0].len(){
            res2[i] += b[i][j] * r[j];
        }
    }

    // Add e2 and x to res
    let mut c2 = vec![Fp::default(); res2.len()];
    for i in 0..res2.len() {
        c2[i] = res2[i] + e2[i] + x[i];
    }

    // Convert the results back to u32 vectors
    let c1: Vec<u32> = convert_to_u32_vector(c1);
    let c2: Vec<u32> = convert_to_u32_vector(c2);

    // Write results to the output
    env::commit(&c1);
    env::commit(&c2);
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
