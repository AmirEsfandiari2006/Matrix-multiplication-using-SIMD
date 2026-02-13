#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

// Data structures aligned for SIMD (SSE requires 16-byte alignment)
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) Vec4;
typedef struct { float m[16]; } __attribute__((aligned(16))) Matrix4;

// External assembly function
extern void multiply_vector_asm(Vec4* out, Matrix4* mat, Vec4* in);

// C implementation of the same logic for benchmarking
void multiply_vector_c(Vec4* out, Matrix4* mat, Vec4* in) {
    out->x = mat->m[0]*in->x + mat->m[1]*in->y + mat->m[2]*in->z + mat->m[3]*in->w;
    out->y = mat->m[4]*in->x + mat->m[5]*in->y + mat->m[6]*in->z + mat->m[7]*in->w;
    out->z = mat->m[8]*in->x + mat->m[9]*in->y + mat->m[10]*in->z + mat->m[11]*in->w;
    out->w = mat->m[12]*in->x + mat->m[13]*in->y + mat->m[14]*in->z + mat->m[15]*in->w;
}

int main() {
    // Screen setup
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Assembly Project - 3D Cube Benchmark");
    SetTargetFPS(60);

    // 1. Define Cube Vertices (8 corners)
    Vec4 cube[8] = {
        {-1, -1,  1, 1}, { 1, -1,  1, 1}, { 1,  1,  1, 1}, {-1,  1,  1, 1},
        {-1, -1, -1, 1}, { 1, -1, -1, 1}, { 1,  1, -1, 1}, {-1,  1, -1, 1}
    };

    // 2. Define Edges (Lines connecting vertices)
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0}, // Front
        {4,5}, {5,6}, {6,7}, {7,4}, // Back
        {0,4}, {1,5}, {2,6}, {3,7}  // Sides
    };

    float angle = 0.0f;
    char benchmark_text[100] = "Running Benchmark...";

    // --- BENCHMARK SECTION ---
    Matrix4 test_mat = { {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1} };
    Vec4 test_vec = {1, 2, 3, 4};
    clock_t start, end;
    int iterations = 1000000;

    // Test C Speed
    start = clock();
    for(int i=0; i<iterations; i++) multiply_vector_c(&test_vec, &test_mat, &test_vec);
    double time_c = (double)(clock() - start) / CLOCKS_PER_SEC;

    // Test ASM SIMD Speed
    start = clock();
    for(int i=0; i<iterations; i++) multiply_vector_asm(&test_vec, &test_mat, &test_vec);
    double time_asm = (double)(clock() - start) / CLOCKS_PER_SEC;

    sprintf(benchmark_text, "C: %.4fs | ASM: %.4fs | Speedup: %.2fx", 
            time_c, time_asm, time_c/time_asm);

    // Main Loop
    while (!WindowShouldClose()) {
        angle += 0.02f;
        float c = cosf(angle);
        float s = sinf(angle);

        // Rotation Matrix (Y and X axis rotation combined)
        Matrix4 rot = {{
            c,    s*s,  s*c,  0,
            0,    c,    -s,   0,
            -s,   c*s,  c*c,  0,
            0,    0,    0,    1
        }};

        BeginDrawing();
        ClearBackground(BLACK);

        // UI Information
        DrawText("3D Cube Rendered via x86_64 SIMD (SSE)", 10, 10, 20, RAYWHITE);
        DrawText(benchmark_text, 10, 40, 18, GREEN);

        Vec4 tv[8]; // Transformed vertices
        for (int i = 0; i < 8; i++) {
            // CALL ASSEMBLY CORE
            multiply_vector_asm(&tv[i], &rot, &cube[i]);
        }

        // Draw Lines with basic Perspective Projection
        for (int i = 0; i < 12; i++) {
            Vec4 p1 = tv[edges[i][0]];
            Vec4 p2 = tv[edges[i][1]];

            // Simple perspective: divide x,y by z (z is shifted to avoid /0)
            float z_offset = 4.0f;
            float f1 = 300.0f / (p1.z + z_offset);
            float f2 = 300.0f / (p2.z + z_offset);

            DrawLine(p1.x * f1 + 400, p1.y * f1 + 300, 
                     p2.x * f2 + 400, p2.y * f2 + 300, LIME);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}