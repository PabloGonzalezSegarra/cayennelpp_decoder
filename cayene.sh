#!/bin/bash

# Script de utilidad para trabajar con el proyecto Cayene Decoder
# Uso: ./cayene.sh [comando] [debug|release]

set -e  # Salir si hay errores

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}"
BUILD_TYPE="${BUILD_TYPE:-release}"

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

function print_usage() {
    echo "Uso: $0 [comando] [debug|release]"
    echo ""
    echo "Comandos disponibles:"
    echo "  configure [debug|release]  - Configurar el proyecto con CMake"
    echo "  build [debug|release]      - Compilar el proyecto"
    echo "  test [debug|release]       - Ejecutar tests"
    echo "  run [debug|release]        - Ejecutar el ejemplo básico"
    echo "  clean                      - Limpiar archivos de build"
    echo "  rebuild [debug|release]    - Limpiar y recompilar"
    echo "  format                     - Formatear código con clang-format"
    echo "  tidy [debug|release]       - Analizar código con clang-tidy"
    echo ""
    echo "Ejemplos:"
    echo "  $0 build release"
    echo "  $0 test debug"
    echo "  $0 run"
}

function configure_project() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Configurando proyecto (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    cmake --preset "$build_type"
    echo -e "${GREEN}✓ Configuración completa${NC}"
}

function build_project() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Compilando proyecto (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    cmake --build "build/$build_type"
    cmake --build "build/$build_type" --target update_compile_commands
    echo -e "${GREEN}✓ Compilación completa${NC}"
}

function test_project() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Ejecutando tests (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    
    # Verificar que el directorio de build existe
    if [ ! -d "build/$build_type" ]; then
        echo -e "${YELLOW}Build no encontrado, compilando primero...${NC}"
        configure_project "$build_type"
        build_project "$build_type"
    fi
    
    # Ejecutar tests usando ctest directamente desde el directorio de build
    cd "build/$build_type"
    ctest --output-on-failure
    
    echo -e "${GREEN}✓ Tests completados${NC}"
}

function run_example() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Ejecutando ejemplo básico (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    
    if [ ! -f "build/$build_type/examples/basic_example" ]; then
        echo -e "${YELLOW}Ejecutable no encontrado, compilando primero...${NC}"
        configure_project "$build_type"
        build_project "$build_type"
    fi
    
    "./build/$build_type/examples/basic_example"
}

function clean_project() {
    echo -e "${YELLOW}Limpiando archivos de build...${NC}"
    cd "$PROJECT_ROOT"
    rm -rf build
    echo -e "${GREEN}✓ Limpieza completa${NC}"
}

function rebuild_project() {
    local build_type="${1:-$BUILD_TYPE}"
    clean_project
    configure_project "$build_type"
    build_project "$build_type"
}

function format_code() {
    echo -e "${YELLOW}Formateando código...${NC}"
    cd "$PROJECT_ROOT"
    find src include examples tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
    echo -e "${GREEN}✓ Código formateado${NC}"
}

function tidy_code() {
    local build_type="${1:-$BUILD_TYPE}"
    echo -e "${YELLOW}Analizando código con clang-tidy (${build_type})...${NC}"
    cd "$PROJECT_ROOT"
    
    if [ ! -f "build/$build_type/compile_commands.json" ]; then
        echo -e "${YELLOW}compile_commands.json no encontrado, configurando primero...${NC}"
        configure_project "$build_type"
    fi
    
    clang-tidy -p "build/$build_type" src/*.cpp
    echo -e "${GREEN}✓ Análisis completo${NC}"
}

# Main
if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

COMMAND=$1
shift

case $COMMAND in
    configure)
        configure_project "$@"
        ;;
    build)
        build_project "$@"
        ;;
    test)
        test_project "$@"
        ;;
    run)
        run_example "$@"
        ;;
    clean)
        clean_project
        ;;
    rebuild)
        rebuild_project "$@"
        ;;
    format)
        format_code
        ;;
    tidy)
        tidy_code
        ;;
    *)
        echo -e "${RED}Error: Comando desconocido '$COMMAND'${NC}"
        echo ""
        print_usage
        exit 1
        ;;
esac
