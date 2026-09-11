#!/usr/bin/env bash
# Compara Histograma + Merge Sort secuencial contra OpenMP.
# Uso: bash docs/bench_histograma.sh
set -e

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SRC_SEQ="$ROOT/secuencial/histograma.c"
SRC_OMP="$ROOT/paralelo/histograma.c"
BIN_SEQ="/tmp/hist_serial_bench"
BIN_OMP="/tmp/hist_omp_bench"
N=1000000
HILOS=(1 2 4 8)
REPS=7

# Ambos usan omp_get_wtime; la version secuencial no usa directivas OpenMP.
if command -v gcc >/dev/null && gcc -fopenmp -xc -E - </dev/null >/dev/null 2>&1; then
  gcc -fopenmp -O2 "$SRC_SEQ" -o "$BIN_SEQ"
  gcc -fopenmp -O2 "$SRC_OMP" -o "$BIN_OMP"
else
  BREW=$(brew --prefix libomp)
  COMMON=(-Xpreprocessor -fopenmp -I"$BREW/include" -L"$BREW/lib" -lomp -O2)
  clang "${COMMON[@]}" "$SRC_SEQ" -o "$BIN_SEQ"
  clang "${COMMON[@]}" "$SRC_OMP" -o "$BIN_OMP"
fi

# Los programas generan CSV, DAT y PNG; se ejecutan fuera del repositorio.
WORK=$(mktemp -d)
cd "$WORK"

echo "=== HISTOGRAMA + MERGE SORT: SECUENCIAL ==="
best_seq=""
for rep in $(seq "$REPS"); do
  t=$(echo "$N" | "$BIN_SEQ" 2>/dev/null | grep "Tiempo total" | grep -oE '[0-9]+\.[0-9]+')
  printf "corrida %-2d %s s\n" "$rep" "$t"
  if [ -z "$best_seq" ]; then best_seq=$t; else best_seq=$(python3 -c "print(min($best_seq,$t))"); fi
done
printf "mejor secuencial: %s s\n\n" "$best_seq"

declare -a T
echo "=== HISTOGRAMA + MERGE SORT: PARALELO OPENMP ==="
for p in "${HILOS[@]}"; do
  best=""
  echo "hilos: $p"
  for rep in $(seq "$REPS"); do
    t=$(echo "$N" | OMP_NUM_THREADS="$p" "$BIN_OMP" 2>/dev/null | grep "Tiempo total" | grep -oE '[0-9]+\.[0-9]+')
    printf "  corrida %-2d %s s\n" "$rep" "$t"
    if [ -z "$best" ]; then best=$t; else best=$(python3 -c "print(min($best,$t))"); fi
  done
  T[$p]=$best
  printf "  mejor: %s s\n" "$best"
done

echo ""
echo "=== COMPARACION: SPEEDUP = Ts/Tp ==="
printf "%-12s %-12s %-10s %-14s\n" "version" "tiempo(s)" "speedup" "eficiencia(%)"
printf "%-12s %-12.6f %-10.2f %-14s\n" "secuencial" "$best_seq" 1 "-"
for p in "${HILOS[@]}"; do
  python3 -c "s=$best_seq/${T[$p]}; print('%-12s %-12.6f %-10.2f %-14.1f' % ('OpenMP-$p', ${T[$p]}, s, 100*s/$p))"
done
