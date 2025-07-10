import torch

from dftracer.logger import dftracer, dft_fn as Profile
import os

print("LIB PATH", os.environ["LD_LIBRARY_PATH"])

log_inst = dftracer.initialize_log(logfile="hip_data.pfw", data_dir=None, process_id=-1)

comp_dft = Profile("COMPUTE")


def check_gpu_availability():
    """Check if GPU is available and print device info"""
    print("PyTorch version:", torch.__version__)
    print("CUDA available:", torch.cuda.is_available())

    if torch.cuda.is_available():
        print("CUDA version:", torch.version.cuda)
        print("Number of GPUs:", torch.cuda.device_count())
        print("Current GPU:", torch.cuda.get_device_name(0))
        device = torch.device("cuda")
    else:
        print("Using CPU")
        device = torch.device("cpu")

    return device


@comp_dft.log
def basic_tensor_operations(device):
    """Demonstrate basic tensor operations on GPU"""
    print("\n=== Basic Tensor Operations ===")

    # Create tensors and move to GPU
    a = torch.randn(1000, 1000).to(device)
    b = torch.randn(1000, 1000).to(device)

    print(f"Tensor a device: {a.device}")
    print(f"Tensor b device: {b.device}")

    # Basic operations
    c = a + b
    d = torch.matmul(a, b)
    e = torch.sum(a, dim=1)

    print(f"Addition result shape: {c.shape}")
    print(f"Matrix multiplication result shape: {d.shape}")
    print(f"Sum along dimension 1 shape: {e.shape}")

    return a, b, c, d


if __name__ == "__main__":
    device = check_gpu_availability()
    a, b, c, d = basic_tensor_operations(device)
    log_inst.finalize()
    with open("hip_data.pfw", "r") as f:
        data = f.read()
        assert "HIP_RUNTIME_API" in data, "HIP_RUNTIME_API not found in log file"
