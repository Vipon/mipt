#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <omp.h>

const int R = 12; // уровень редукции
const int numThreads = 4;
const double bmin = 0;
const double bmax = 1;
const double bstep = 0.1;
const double eps = 0.000001;

double norm(double *y, int N)
{
    double n = 0.0;
    for(int i = 0; i < N+1; ++i) {
        n += abs(y[i]);
    }
    return n;
}

void sweep(double p)
{
    int N = pow(2,R);   // количество точек
    double h = 1.0/N;   // шаг сетки

    std::ofstream of("b2.csv",std::fstream::app);
    of << "b = " << p << '\n';

    double **a = new double* [R];
    double **b = new double* [R];
    double **c = new double* [R];
    double **g = new double* [R];
    for(int i = 0; i < R; ++i) {
        a[i] = new double[N+1];
        b[i] = new double[N+1];
        c[i] = new double[N+1];
        g[i] = new double[N+1];
    }

    g[0][0] = 0;
    g[0][N] = 0;

    // начальное приближение удовлетворяющее граничным условиям
    double *y = new double[N+1];
    for(int i = 0; i < N+1; ++i)
        y[i] = 1 + (p - 1)*i*h;

    // поправка к приближению
    double *v = new double[N+1];
    v[0] = 0;
    v[N] = 0;

    int *offset = new int[R];
    for(int i = 0; i < R; ++i)
        offset[i] = (int) pow(2,i);

    double time = omp_get_wtime();
#pragma omp parallel
{
    do {
    // коэффициэнты не редуцированной прогонки
    #pragma omp for
        for(int i = 1; i < N; ++i) {
            a[0][i] = 1 + (h*h) / 12 * exp(-y[i-1]);
            b[0][i] = -2 + 5 / 6 * (h*h) * exp(-y[i]);
            c[0][i] = 1 + (h*h) / 12 * exp(-y[i+1]);
            g[0][i] = (h*h) * (exp(-y[i+1]) + 10*exp(-y[i]) + exp(-y[i-1])) / 12 - (y[i-1] - 2*y[i] + y[i+1]);
        }

    // редукция
        for(int i = 1; i < R; ++i) {
            int off = offset[i];

        #pragma omp for
            for(int j = off; j < N; j += off) {
                a[i][j] = -a[i-1][j] * a[i-1][j-off/2] / b[i-1][j-off/2];
                b[i][j] = b[i-1][j] - a[i-1][j] * c[i-1][j-off/2] / b[i-1][j-off/2] - c[i-1][j] * a[i-1][j+off/2] / b[i-1][j+off/2];
                c[i][j] = -c[i-1][j] * c[i-1][j+off/2] / b[i-1][j+off/2];
                g[i][j] = g[i-1][j] - a[i-1][j] * g[i-1][j-off/2] / b[i-1][j-off/2] - c[i-1][j] * g[i-1][j+off/2] / b[i-1][j+off/2];
            }
        }

    // обратная редукция
        for(int i = R-1; i >= 0; --i) {
            int off = offset[i];

        #pragma omp for
            for(int j = off; j < N; j += 2 * off)
                v[j] = (g[i][j] - c[i][j] * v[j+off] - a[i][j] * v[j-off]) / b[i][j];
        }

    #pragma omp for
        for(int i = 0; i <= N; i++)
            y[i] += v[i];

    } while ( norm(v, N) > eps );
}

    time = omp_get_wtime() - time;
    of << "Time:\t" << time << std::endl;

    of << "X" << "\t\t" << "Y" << std::endl;
    of.precision(7);
    for (int i = 0; i <= N; ++i)
        of << std::fixed << ((double) i / N) << "; " << y[i] << std::endl;

    for (int i = 0; i < R; i++) {
        delete [] a[i];
        delete [] b[i];
        delete [] c[i];
        delete [] g[i];
    }
    delete [] a;
    delete [] b;
    delete [] c;
    delete [] g;
    delete [] y;
    delete [] v;

    of << '\n';
    of.close();
}

int main(int argc, char *argv[])
{
    std::ofstream("b2.csv");
    omp_set_num_threads(numThreads);

    for(double b = bmin; b <= bmax; b += bstep)
        sweep(b);

    return 0;
}

