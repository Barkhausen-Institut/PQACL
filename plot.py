import matplotlib.pyplot as plt
import numpy as np
import json

# Set font for scientific publication
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Times New Roman', 'Times', 'DejaVu Serif', 'serif']
plt.rcParams['font.size'] = 24
plt.rcParams['axes.labelsize'] = 24
plt.rcParams['axes.titlesize'] = 28
plt.rcParams['legend.fontsize'] = 22
plt.rcParams['xtick.labelsize'] = 20
plt.rcParams['ytick.labelsize'] = 20
plt.rcParams['figure.titlesize'] = 32

# dict: n_m_l_k_KEY : VALUE
# n: number of files
N = [64, 128, 256, 512, 1024]
# m: number of secrets
M = [2, 4, 8, 16, 32, 64]
# l: PVW redandancy
L = [10, 20, 30, 40, 50]
# k: LWE redundancy
K = [1, 2, 3, 4, 5]
KEY = ["ZKP_BP_Dec_Prove", 
       "ZKP_BP_Enc_Prove", 
       "ZKP_BP_Enc_Verify", 
       "ZKP_BP_Dec_Verify", 
       "ZKP_STARK_Dec_Prove",
       "ZKP_STARK_Enc_Prove",
       "ZKP_STARK_Enc_Verify",
       "ZKP_STARK_Dec_Verify"]
def load_dataset(dir):
    data = {}
    for n in N:
        for m in M:
            for l in L:
                for k in K:
                    file_path = f"{dir}/Files_{n}_size_{m}_redundancyEnc_{l}_redundancyLWE_{k}.json"
                    try:
                        file_obj = open(file_path, 'r')
                        j = json.load(file_obj)
                        for key in KEY:
                            if key in j:
                                data[f"{n}_{m}_{l}_{k}_{key}"] = j[key]
                        file_obj.close()
                    except FileNotFoundError:
                        continue
    return data

def plot_prover(data):
    # Create a figure with 4 subplots (2x2 grid)
    fig, axes = plt.subplots(2, 2, figsize=(20, 10))
    # fig.suptitle('ZK Proof Performance', fontsize=24)
    
    # Define the four ZK proof methods
    methods = ["ZKP_BP_Enc_Prove", "ZKP_BP_Dec_Prove", "ZKP_STARK_Enc_Prove", "ZKP_STARK_Dec_Prove"]
    colors = ['blue', 'red', 'green', 'orange']
    
    # Figure a: m=32, l=20, k=2, n in N
    ax = axes[0, 0]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for n in N:
            key = f"{n}_32_20_2_{method}"
            if key in data:
                x_vals.append(n)
                y_vals.append(data[key])
        if x_vals:
            y_vals_seconds = [y/1000 for y in y_vals]  # Convert ms to seconds
            ax.plot(x_vals, y_vals_seconds, marker='o', color=color, label=method.replace("ZKP_", ""), linewidth=2)
    ax.set_title('Figure a: m=32, l=20, k=2')
    ax.set_xlabel('Number of Files (n)')
    ax.set_ylabel('Time (s)')
    ax.grid(True, alpha=0.3)
    
    # Figure b: n=512, l=20, k=2, m in M
    ax = axes[0, 1]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for m in M:
            key = f"512_{m}_20_2_{method}"
            if key in data:
                x_vals.append(m)
                y_vals.append(data[key])
        if x_vals:
            y_vals_seconds = [y/1000 for y in y_vals]  # Convert ms to seconds
            ax.plot(x_vals, y_vals_seconds, marker='o', color=color, linewidth=2)
    ax.set_title('Figure b: n=512, l=20, k=2')
    ax.set_xlabel('Number of Secrets (m)')
    ax.set_ylabel('Time (s)')
    ax.grid(True, alpha=0.3)
    
    # Figure c: n=512, m=32, k=2, l in L
    ax = axes[1, 0]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for l in L:
            key = f"512_32_{l}_2_{method}"
            if key in data:
                x_vals.append(l)
                y_vals.append(data[key])
        if x_vals:
            y_vals_seconds = [y/1000 for y in y_vals]  # Convert ms to seconds
            ax.plot(x_vals, y_vals_seconds, marker='o', color=color, linewidth=2)
    ax.set_title('Figure c: n=512, m=32, k=2')
    ax.set_xlabel('PVW Redundancy (l)')
    ax.set_ylabel('Time (s)')
    ax.grid(True, alpha=0.3)
    
    # Figure d: n=512, m=32, l=20, k in K
    ax = axes[1, 1]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for k in K:
            key = f"512_32_20_{k}_{method}"
            if key in data:
                x_vals.append(k)
                y_vals.append(data[key])
        if x_vals:
            y_vals_seconds = [y/1000 for y in y_vals]  # Convert ms to seconds
            ax.plot(x_vals, y_vals_seconds, marker='o', color=color, linewidth=2)
    ax.set_title('Figure d: n=512, m=32, l=20')
    ax.set_xlabel('LWE Redundancy (k)')
    ax.set_ylabel('Time (s)')
    ax.grid(True, alpha=0.3)
    
    # Add a single legend for the entire figure
    handles, labels = axes[0, 0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, -0.035), ncol=4)
    
    plt.tight_layout()
    
    # Save to PDF
    plt.savefig('zk_proof.pdf', bbox_inches='tight', dpi=300)
    plt.show()

def plot_verifier(data):
    # Create a figure with 4 subplots (2x2 grid)
    fig, axes = plt.subplots(2, 2, figsize=(20, 10))
    # fig.suptitle('ZK Verification Performance', fontsize=24)
    
    # Define the four ZK proof verification methods
    methods = ["ZKP_BP_Enc_Verify", "ZKP_BP_Dec_Verify", "ZKP_STARK_Enc_Verify", "ZKP_STARK_Dec_Verify"]
    colors = ['blue', 'red', 'green', 'orange']
    
    # Figure a: m=32, l=20, k=2, n in N
    ax = axes[0, 0]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for n in N:
            key = f"{n}_32_20_2_{method}"
            if key in data:
                x_vals.append(n)
                y_vals.append(data[key])
        if x_vals:
            ax.plot(x_vals, y_vals, marker='o', color=color, label=method.replace("ZKP_", ""), linewidth=2)
    ax.set_title('Figure a: m=32, l=20, k=2')
    ax.set_xlabel('Number of Files (n)')
    ax.set_ylabel('Time (ms)')
    ax.grid(True, alpha=0.3)
    
    # Figure b: n=512, l=20, k=2, m in M
    ax = axes[0, 1]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for m in M:
            key = f"512_{m}_20_2_{method}"
            if key in data:
                x_vals.append(m)
                y_vals.append(data[key])
        if x_vals:
            ax.plot(x_vals, y_vals, marker='o', color=color, linewidth=2)
    ax.set_title('Figure b: n=512, l=20, k=2')
    ax.set_xlabel('Number of Secrets (m)')
    ax.set_ylabel('Time (ms)')
    ax.grid(True, alpha=0.3)
    
    # Figure c: n=512, m=32, k=2, l in L
    ax = axes[1, 0]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for l in L:
            key = f"512_32_{l}_2_{method}"
            if key in data:
                x_vals.append(l)
                y_vals.append(data[key])
        if x_vals:
            ax.plot(x_vals, y_vals, marker='o', color=color, linewidth=2)
    ax.set_title('Figure c: n=512, m=32, k=2')
    ax.set_xlabel('PVW Redundancy (l)')
    ax.set_ylabel('Time (ms)')
    ax.grid(True, alpha=0.3)
    
    # Figure d: n=512, m=32, l=20, k in K
    ax = axes[1, 1]
    for method, color in zip(methods, colors):
        x_vals = []
        y_vals = []
        for k in K:
            key = f"512_32_20_{k}_{method}"
            if key in data:
                x_vals.append(k)
                y_vals.append(data[key])
        if x_vals:
            ax.plot(x_vals, y_vals, marker='o', color=color, linewidth=2)
    ax.set_title('Figure d: n=512, m=32, l=20')
    ax.set_xlabel('LWE Redundancy (k)')
    ax.set_ylabel('Time (ms)')
    ax.grid(True, alpha=0.3)
    
    # Add a single legend for the entire figure
    handles, labels = axes[0, 0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, -0.035), ncol=4)
    
    plt.tight_layout()
    
    # Save to PDF
    plt.savefig('zk_verif.pdf', bbox_inches='tight', dpi=300)
    plt.show()

if __name__ == "__main__":
    dir = "logs/"
    data = load_dataset(dir)
    print(f"Loaded {len(data)} data points")
    plot_prover(data)
    plot_verifier(data)
