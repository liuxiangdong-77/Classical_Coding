#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cuda_runtime.h>

__global__ void vectorAdd(const float* a, const float* b, float* c, int n){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < n) c[i] = a[i] + b[i];
}

int main(){
  const int n = 1 << 20;
  std::vector<float> h_a(n, 1.0f), h_b(n, 2.0f), h_c(n, 0.0f);

  int nDev = 0;
  cudaGetDeviceCount(&nDev);
  const int chunk = (n + nDev - 1) / nDev;
  std::vector<float*> d_a(nDev), d_b(nDev), d_c(nDev);

  for (int d = 0; d < nDev; ++d){
    const int off = d * chunk;
    const int cnt = std::min(chunk, n - off);
    const size_t bytes = cnt * sizeof(float);

    cudaSetDevice(d);
    cudaMalloc(&d_a[d], bytes);
    cudaMalloc(&d_b[d], bytes);
    cudaMalloc(&d_c[d], bytes);
    cudaMemcpy(d_a[d], h_a.data() + off, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b[d], h_b.data() + off, bytes, cudaMemcpyHostToDevice);
    vectorAdd<<<(cnt + 255) / 256, 256>>>(d_a[d], d_b[d], d_c[d], cnt);
    printf("GPU %d, [%d, %d)\n", d, off, off + cnt);
  }

  for (int d = 0; d < nDev; ++d){
    const int off = d * chunk;
    const int cnt = std::min(chunk, n - off);
    const int bytes = cnt * sizeof(float);

    cudaSetDevice(d);
    cudaMemcpy(h_c.data() + off, d_c[d], bytes, cudaMemcpyDeviceToHost);
    cudaFree(d_a[d]);
    cudaFree(d_b[d]);
    cudaFree(d_c[d]);
  }

  bool ok = true;
  for(int i = 0; i < n; ++i) {
    if(h_c[i] != 3.0f){
      ok = false;
      break;
    }
  }
  printf("%d GPU(s), c[0] = %f, c[n-1] = %f -> %s\n", nDev, h_c[0], h_c[n-1], ok ? "all correct (3.0)" : "WRONG result!");
  return 0;
}