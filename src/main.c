#include "raylib.h"
#include <math.h>
#include <stdio.h>

// --- Project Configuration ---
#define STRESS_TEST_ITERATIONS 100000
#define SCREEN_WIDTH  960
#define SCREEN_HEIGHT 600
#define CUBE_DISTANCE 4.0f

// --- Chart Configuration ---
#define HISTORY_SIZE 200
#define CHART_WIDTH  600
#define CHART_HEIGHT 120
#define CHART_X      200
#define CHART_Y      400

// --- SIMD Aligned Data Structures ---
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) Vec4;
typedef struct { float m[16]; } __attribute__((aligned(16))) Matrix4;

// --- Scene Graph Node ---
typedef struct {
    Matrix4 localTransform;
    Vec4* vertices;
    int vertexCount;
} SceneNode;

// --- External ASM SIMD Function ---
extern void multiply_vector_asm(Vec4* out, Matrix4* mat, Vec4* in);

// --- Scalar Reference Implementation ---
void multiply_vector_c(Vec4* out, Matrix4* mat, Vec4* in) {
    out->x = mat->m[0]*in->x + mat->m[1]*in->y + mat->m[2]*in->z + mat->m[3]*in->w;
    out->y = mat->m[4]*in->x + mat->m[5]*in->y + mat->m[6]*in->z + mat->m[7]*in->w;
    out->z = mat->m[8]*in->x + mat->m[9]*in->y + mat->m[10]*in->z + mat->m[11]*in->w;
    out->w = mat->m[12]*in->x + mat->m[13]*in->y + mat->m[14]*in->z + mat->m[15]*in->w;
}

int main() {

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Assembly Game Engine - SIMD Benchmark");
    SetTargetFPS(60);

    // Cube vertices
    Vec4 cubeVertices[8] = {
        {-1,-1, 1, 1}, { 1,-1, 1, 1}, { 1, 1, 1, 1}, {-1, 1, 1, 1},
        {-1,-1,-1, 1}, { 1,-1,-1, 1}, { 1, 1,-1, 1}, {-1, 1,-1, 1}
    };

    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    float angle = 0.0f;
    double avgTimeAsm = 0, avgTimeC = 0;
    int frames = 0;

    // --- History buffers ---
    float asmHistory[HISTORY_SIZE] = {0};
    float cHistory[HISTORY_SIZE]   = {0};
    int historyIndex = 0;

    while (!WindowShouldClose()) {

        angle += 0.02f;

        float c = cosf(angle);
        float s = sinf(angle);

        Matrix4 rotMat = {{
            c, 0, s, 0,
            0, 1, 0, 0,
           -s, 0, c, 0,
            0, 0, 0, 1
        }};

        // --- SIMD Benchmark ---
        double startAsm = GetTime();
        Vec4 tvAsm[8];
        for(int j=0; j < STRESS_TEST_ITERATIONS; j++)
            for(int i=0; i<8; i++)
                multiply_vector_asm(&tvAsm[i], &rotMat, &cubeVertices[i]);
        double endAsm = GetTime();

        // --- Scalar Benchmark ---
        double startC = GetTime();
        Vec4 tvC[8];
        for(int j=0; j < STRESS_TEST_ITERATIONS; j++)
            for(int i=0; i<8; i++)
                multiply_vector_c(&tvC[i], &rotMat, &cubeVertices[i]);
        double endC = GetTime();

        double frameTimeAsm = (endAsm - startAsm) * 1000.0;
        double frameTimeC   = (endC - startC) * 1000.0;

        if (++frames >= 20) {
            avgTimeAsm = frameTimeAsm;
            avgTimeC   = frameTimeC;

            asmHistory[historyIndex] = avgTimeAsm;
            cHistory[historyIndex]   = avgTimeC;

            historyIndex = (historyIndex + 1) % HISTORY_SIZE;
            frames = 0;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // --- Performance Text ---
        DrawText("ASM (SIMD) CORE", 150, 40, 20, LIME);
        DrawText(TextFormat("%.3f ms", avgTimeAsm), 180, 70, 18, WHITE);

        DrawText("C (SCALAR) CORE", 650, 40, 20, RED);
        DrawText(TextFormat("%.3f ms", avgTimeC), 680, 70, 18, WHITE);

        if (avgTimeAsm > 0.0)
            DrawText(TextFormat("SPEEDUP: %.2fx", avgTimeC/avgTimeAsm), 430, 560, 22, YELLOW);

        // --- Render Cubes ---
        for (int i = 0; i < 12; i++) {

            float f1 = 200.0f / (tvAsm[edges[i][0]].z + CUBE_DISTANCE);
            float f2 = 200.0f / (tvAsm[edges[i][1]].z + CUBE_DISTANCE);

            DrawLine(
                tvAsm[edges[i][0]].x * f1 + 250,
                tvAsm[edges[i][0]].y * f1 + 300,
                tvAsm[edges[i][1]].x * f2 + 250,
                tvAsm[edges[i][1]].y * f2 + 300,
                LIME
            );

            float f3 = 200.0f / (tvC[edges[i][0]].z + CUBE_DISTANCE);
            float f4 = 200.0f / (tvC[edges[i][1]].z + CUBE_DISTANCE);

            DrawLine(
                tvC[edges[i][0]].x * f3 + 750,
                tvC[edges[i][0]].y * f3 + 300,
                tvC[edges[i][1]].x * f4 + 750,
                tvC[edges[i][1]].y * f4 + 300,
                RED
            );
        }

        // =============================
        //         BENCHMARK CHART
        // =============================

        DrawRectangleLines(CHART_X, CHART_Y, CHART_WIDTH, CHART_HEIGHT, GRAY);
        DrawText("Performance History (ms)", CHART_X + 180, CHART_Y - 20, 16, WHITE);

        float maxValue = 0.0f;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (asmHistory[i] > maxValue) maxValue = asmHistory[i];
            if (cHistory[i] > maxValue)   maxValue = cHistory[i];
        }
        if (maxValue < 0.1f) maxValue = 0.1f;

        for (int i = 1; i < HISTORY_SIZE; i++) {

            int idx1 = (historyIndex + i - 1) % HISTORY_SIZE;
            int idx2 = (historyIndex + i) % HISTORY_SIZE;

            float x1 = CHART_X + ((float)(i - 1) / HISTORY_SIZE) * CHART_WIDTH;
            float x2 = CHART_X + ((float)i / HISTORY_SIZE) * CHART_WIDTH;

            float yAsm1 = CHART_Y + CHART_HEIGHT - (asmHistory[idx1] / maxValue) * CHART_HEIGHT;
            float yAsm2 = CHART_Y + CHART_HEIGHT - (asmHistory[idx2] / maxValue) * CHART_HEIGHT;

            float yC1 = CHART_Y + CHART_HEIGHT - (cHistory[idx1] / maxValue) * CHART_HEIGHT;
            float yC2 = CHART_Y + CHART_HEIGHT - (cHistory[idx2] / maxValue) * CHART_HEIGHT;

            DrawLine(x1, yAsm1, x2, yAsm2, LIME);
            DrawLine(x1, yC1,   x2, yC2,   RED);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
