/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void trans_32(int M, int N, int A[N][M], int B[M][N]);
void trans_64(int M, int N, int A[N][M], int B[M][N]);
void trans_61(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch(M)
    {
        case 32:
            trans_32(M,N,A,B);
            break;
        case 64:
            trans_64(M,N,A,B);
            break;
        case 61:
            trans_61(M,N,A,B);
            break;
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

char trans_32_desc[] = "32 Transpose ";
void trans_32(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i < N; i += 8)
    {
        for (int j = 0; j < M; j += 8)
        {
            int n;
            int m;
            int a0;
            int a1;
            int a2;
            int a3;
            int a4;
            int a5;
            int a6;
            int a7;
            //block transpose
            for (n = i, m = j; n < i + 8; n++, m++)
            {
                a0 = A[n][j];
                a1 = A[n][j + 1];
                a2 = A[n][j + 2];
                a3 = A[n][j + 3];
                a4 = A[n][j + 4];
                a5 = A[n][j + 5];
                a6 = A[n][j + 6];
                a7 = A[n][j + 7];
                
                B[m][i] = a0;
                B[m][i + 1] = a1;
                B[m][i + 2] = a2;
                B[m][i + 3] = a3;
                B[m][i + 4] = a4;
                B[m][i + 5] = a5;
                B[m][i + 6] = a6;
                B[m][i + 7] = a7;
                
                
            }
            //elem transpose
            for (m = 0; m < 8; m++)
            {
                for (n = m + 1; n < 8; n++)
                {
                    a0 = B[j + m][i + n];
                    B[j + m][i + n] = B[j + n][i + m];
                    B[j + n][i + m] = a0;
                }
            }
        }
    }
}


char trans_64_desc[] = "64 Transpose";
void trans_64(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i < N; i += 8)
    {
        for (int j = 0; j < M; j += 8)
        {
            int n;
            int temp;
            int a0;
            int a1;
            int a2;
            int a3;
            int a4;
            int a5;
            int a6;
            int a7;
            
            for (n = 0; n < 4; n++)
            {
                a0 = A[i + n][j];
                a1 = A[i + n][j + 1];
                a2 = A[i + n][j + 2];
                a3 = A[i + n][j + 3];
                a4 = A[i + n][j + 4];
                a5 = A[i + n][j + 5];
                a6 = A[i + n][j + 6];
                a7 = A[i + n][j + 7];
                
                B[j]    [i + n] = a0;
                B[j + 1][i + n] = a1;
                B[j + 2][i + n] = a2;
                B[j + 3][i + n] = a3;
                
                B[j]    [i + n + 4] = a4;
                B[j + 1][i + n + 4] = a5;
                B[j + 2][i + n + 4] = a6;
                B[j + 3][i + n + 4] = a7;
            }
            
            for (n = 0; n < 4; n++)
            {
                a0 = A[i + 4][j + n];
                a1 = A[i + 5][j + n];
                a2 = A[i + 6][j + n];
                a3 = A[i + 7][j + n];
                
                a4 = A[i + 4][j + n + 4];
                a5 = A[i + 5][j + n + 4];
                a6 = A[i + 6][j + n + 4];
                a7 = A[i + 7][j + n + 4];
                
                temp = B[j + n][i + 4];
                B[j + n][i + 4] = a0;
                a0 = temp;
                temp = B[j + n][i + 5];
                B[j + n][i + 5] = a1;
                a1 = temp;
                temp = B[j + n][i + 6];
                B[j + n][i + 6] = a2;
                a2 = temp;
                temp = B[j + n][i + 7];
                B[j + n][i + 7] = a3;
                a3 = temp;
                
                B[j + n + 4][i]     = a0;
                B[j + n + 4][i + 1] = a1;
                B[j + n + 4][i + 2] = a2;
                B[j + n + 4][i + 3] = a3;
                B[j + n + 4][i + 4] = a4;
                B[j + n + 4][i + 5] = a5;
                B[j + n + 4][i + 6] = a6;
                B[j + n + 4][i + 7] = a7;
            }
        }
    }
}

char trans_61_desc[] = "61 Transpose";
void trans_61(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i < N; i += 16)
    {
        for (int j = 0; j < M; j += 16)
        {
            for (int n = i; n < i + 16 && n < N; n++)
            {
                for (int m = j; m < j + 16 && m < M; m++)
                    B[m][n] = A[n][m];
            }
        }
    }
}

char my_trans_64_desc[] = "my 64 Transpose";
void my_trans_64(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i < N; i += 8)
    {
        for (int j = 0; j < M; j += 8)
        {
            int n;
            int a0;
            int a1;
            int a2;
            int a3;
            int a4;
            int a5;
            int a6;
            int a7;
            
            for (n = 0; n < 4; n++)
            {
                a0 = A[i + n][j];
                a1 = A[i + n][j + 1];
                a2 = A[i + n][j + 2];
                a3 = A[i + n][j + 3];
                a4 = A[i + n][j + 4];
                a5 = A[i + n][j + 5];
                a6 = A[i + n][j + 6];
                a7 = A[i + n][j + 7];
                
                B[j]    [i + n] = a0;
                B[j + 1][i + n] = a1;
                B[j + 2][i + n] = a2;
                B[j + 3][i + n] = a3;
                
                B[j]    [i + n + 4] = a4;
                B[j + 1][i + n + 4] = a5;
                B[j + 2][i + n + 4] = a6;
                B[j + 3][i + n + 4] = a7;
            }
            
            for (n = 4; n < 8; n++)
            {
                a0 = A[i + n][j];
                a1 = A[i + n][j + 1];
                a2 = A[i + n][j + 2];
                a3 = A[i + n][j + 3];
                a4 = A[i + n][j + 4];
                a5 = A[i + n][j + 5];
                a6 = A[i + n][j + 6];
                a7 = A[i + n][j + 7];
                
                B[j + 4][i + n - 4] = a0;
                B[j + 5][i + n - 4] = a1;
                B[j + 6][i + n - 4] = a2;
                B[j + 7][i + n - 4] = a3;
                
                B[j + 4][i + n] = a4;
                B[j + 5][i + n] = a5;
                B[j + 6][i + n] = a6;
                B[j + 7][i + n] = a7;
            }
            
            for (n = 0; n < 4; n++)
            {
                a0 = B[j + 4 + n][i];
                a1 = B[j + 4 + n][i + 1];
                a2 = B[j + 4 + n][i + 2];
                a3 = B[j + 4 + n][i + 3];
                
                a4 = B[j + n][i + 4];
                a5 = B[j + n][i + 5];
                a6 = B[j + n][i + 6];
                a7 = B[j + n][i + 7];
                
                B[j + n][i + 4] = a0;
                B[j + n][i + 5] = a1;
                B[j + n][i + 6] = a2;
                B[j + n][i + 7] = a3;
                
                B[j + 4 + n][i]     = a4;
                B[j + 4 + n][i + 1] = a5;
                B[j + 4 + n][i + 2] = a6;
                B[j + 4 + n][i + 3] = a7;
            }
        }
    }
}

char solve_64_desc[] = "solve_64";
void solve_64(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i < N; i += 8)
    {
        for (int j = 0; j < M; j += 8)
        {
            int n;
            int a0;
            int a1;
            int a2;
            int a3;
            int a4;
            int a5;
            int a6;
            int a7;
            
            for (n = 0; n < 4; n++)
            {
                a0 = A[i + n][j];
                a1 = A[i + n][j + 1];
                a2 = A[i + n][j + 2];
                a3 = A[i + n][j + 3];
                a4 = A[i + n][j + 4];
                a5 = A[i + n][j + 5];
                a6 = A[i + n][j + 6];
                a7 = A[i + n][j + 7];
                
                B[j]    [i + n] = a0;
                B[j + 1][i + n] = a1;
                B[j + 2][i + n] = a2;
                B[j + 3][i + n] = a3;
                
                B[j]    [i + n + 4] = a4;
                B[j + 1][i + n + 4] = a5;
                B[j + 2][i + n + 4] = a6;
                B[j + 3][i + n + 4] = a7;
            }
            
            for (n = 0; n < 4; n++)
            {
                a0 = A[i + 4][j + n];
                a1 = A[i + 5][j + n];
                a2 = A[i + 6][j + n];
                a3 = A[i + 7][j + n];
                
                a4 = B[j + n][i + 4];
                a5 = B[j + n][i + 5];
                a6 = B[j + n][i + 6];
                a7 = B[j + n][i + 7];
                
                B[j + n][i + 4] = a0;
                B[j + n][i + 5] = a1;
                B[j + n][i + 6] = a2;
                B[j + n][i + 7] = a3;
                
                B[j + n + 4][i]     = a4;
                B[j + n + 4][i + 1] = a5;
                B[j + n + 4][i + 2] = a6;
                B[j + n + 4][i + 3] = a7;
                
            }
            
            for (n = 0; n < 4; n++)
            {
                a0 = A[i + 4][j + n + 4];
                a1 = A[i + 5][j + n + 4];
                a2 = A[i + 6][j + n + 4];
                a3 = A[i + 7][j + n + 4];
                
                B[j + n + 4][i + 4] = a0;
                B[j + n + 4][i + 5] = a1;
                B[j + n + 4][i + 6] = a2;
                B[j + n + 4][i + 7] = a3;
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    
    registerTransFunction(solve_64, solve_64_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

