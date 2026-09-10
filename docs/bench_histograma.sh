#!/usr/bin/env bash
# Benchmark del Histograma paralelo (OpenMP).
# Mide el tiempo TOTAL del programa (merge sort + histograma) vs numero de hilos
# y calcula speedup y eficiencia. El histograma por si solo (~2 ms) es demasiado
# rapido para medirlo de forma confiable, por eso se usa el tiempo total.
# Uso:  bash docs/bench_histograma.sh
set -e

SRC="paralelo/histograma.c"
BIN="/tmp/hist_omp_bench"
N=1000000              # maximo que permite el codigo (#define MAX 1000000)
HILOS=(1 2 4 8 12)
REPS=7                 # corridas por config; se toma la mejor

# --- Compilar (Mac con libomp o Linux con gcc) ---
if command -v gcc >/dev/null && gcc -fopenmp -xc -E - </dev/null >/dev/null 2>&1; then
  gcc -fopenmp -O2 "$SRC" -o "$BIN"
else
  BREW=$(brew --prefix libomp)
  clang -Xpreprocessor -fopenmp -I"$BREW/include" -L"$BREW/lib" -lomp -O2 "$SRC" -o "$BIN"
fi

# El programa genera archivos (csv/dat/png) en el directorio actual:
# lo corremos en una carpeta temporal para no ensuciar el repo.
WORK=$(mktemp -d)
cd "$WORK"

# El numero de hilos se controla con OMP_NUM_THREADS; N se pasa por stdin.
declare -a T
echo "hilos  total(s)"
for p in "${HILOS[@]}"; do
  best=""
  for _ in $(seq "$REPS"); do
    t=$(echo "$N" | OMP_NUM_THREADS="$p" "$BIN" 2>/dev/null | grep "Tiempo total" | grep -oE '[0-9]+\.[0-9]+')
    if [ -z "$best" ]; then best=$t; else best=$(python3 -c "print(min($best,$t))"); fi
  done
  T[$p]=$best
  printf "%-6d %s\n" "$p" "$best"
done

echo ""
printf "%-6s %-12s %-10s %-12s\n" "hilos" "total(s)" "speedup" "eficiencia(%)"
T1=${T[1]}
for p in "${HILOS[@]}"; do
  python3 -c "s=$T1/${T[$p]}; print('%-6d %-12.5f %-10.2f %-12.1f' % ($p, ${T[$p]}, s, 100*s/$p))"
done
