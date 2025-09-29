#!/usr/bin/env bash

# defaults: 512 32 20 2
# n 64 128 256 512 1024
for n in 512; do
    # m 2 4 8 16 32 64
    for m in 32; do
        # l 10 20 30 40 50
        for l in 20; do
            # k 1 2 3 4 5
            for k in 2; do
                echo "Running benchmark with parameters: n=$n, m=$m, l=$l, k=$k"
                mkdir -p logs
                log_name="Files_${n}_size_${m}_redundancyEnc_${l}_redundancyLWE_${k}.json"
                log_file="logs/$log_name"
                touch enc1_proof.bin
                ./AccessControl $n $m $l $k
                mv $log_name $log_file
                t=$(ZKP/zkstark_enc_prover $m $l $k enc1_prover.json tmp.bin | cut -d" " -f5); t=($t)
                sed -i "s/.*ZKP_STARK_Enc_Prove.*/    \"ZKP_STARK_Enc_Prove\": ${t[0]},/" $log_file
                sed -i "s/.*ZKP_STARK_Enc_Size.*/    \"ZKP_STARK_Enc_Size\": ${t[1]},/" $log_file
                t=$(ZKP/zkstark_enc_verifier $m $l $k enc1_verifier.json tmp.bin | cut -d" " -f5)
                sed -i "s/.*ZKP_STARK_Enc_Verify.*/    \"ZKP_STARK_Enc_Verify\": $t,/" $log_file
                t=$(ZKP/zkstark_dec_prover $m $l $k ds1_data/dec_prover.json tmp.bin | cut -d" " -f5); t=($t)
                sed -i "s/.*ZKP_STARK_Dec_Prove.*/    \"ZKP_STARK_Dec_Prove\": ${t[0]},/" $log_file
                sed -i "s/.*ZKP_STARK_Dec_Size.*/    \"ZKP_STARK_Dec_Size\": ${t[1]},/" $log_file
                t=$(ZKP/zkstark_dec_verifier $m $l $k ds1_data/dec_verifier.json tmp.bin | cut -d" " -f5)
                sed -i "s/.*ZKP_STARK_Dec_Verify.*/    \"ZKP_STARK_Dec_Verify\": $t,/" $log_file
                t=$(ZKP/zkbp_enc $m $l $k enc1_prover.json tmp.bin | cut -d" " -f5); t=($t)
                sed -i "s/.*ZKP_BP_Enc_Prove.*/    \"ZKP_BP_Enc_Prove\": ${t[0]},/" $log_file
                sed -i "s/.*ZKP_BP_Enc_Verify.*/    \"ZKP_BP_Enc_Verify\": ${t[1]},/" $log_file
                sed -i "s/.*ZKP_BP_Enc_Size.*/    \"ZKP_BP_Enc_Size\": ${t[2]},/" $log_file
                t=$(ZKP/zkbp_dec $m $l $k ds1_data/dec_prover.json tmp.bin | cut -d" " -f5); t=($t)
                sed -i "s/.*ZKP_BP_Dec_Prove.*/    \"ZKP_BP_Dec_Prove\": ${t[0]},/" $log_file
                sed -i "s/.*ZKP_BP_Dec_Verify.*/    \"ZKP_BP_Dec_Verify\": ${t[1]},/" $log_file
                sed -i "s/.*ZKP_BP_Dec_Size.*/    \"ZKP_BP_Dec_Size\": ${t[2]},/" $log_file
            done
        done
    done
done