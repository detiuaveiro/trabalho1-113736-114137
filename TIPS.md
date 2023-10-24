# Projecto 1


## Ficheiros fornecidos

- `image8bit.h` - interface do módulo
- `image8bit.c` - implementação do módulo (a COMPLETAR)
- `imageTest.c` - programa de teste simples
- `imageTool.c` - programa de teste mais versátil
- `Makefile`    - regras para compilar usando `make`


## Sugestões para o desenvolvimento

Sugere-se o desenvolvimento progressivo pela seguinte ordem:

1. Completar `ImageCreate`.
2. Completar `ImageDestroy`.
3. Compilar os programas de teste correndo `make`.
4. Testar: `./imageTool einstein.pgm save out.pgm`.
   Isto chama `ImageLoad`, que chama `ImageCreate`
   e depois `ImageSave` e `ImageDestroy`.
5. Completar a função interna `G`.
6. Testar com `ImageSetPixel` e `ImageGePixel`.
7. Fazer `ImageStats`
8. Fazer `ImageNegative`, `ImageThreshold`, `ImageBrighten`.
9. Fazer `ImageValidRect`.
10. Fazer `ImageMirror`, `ImageRotate`.
11. Fazer `ImageCrop`, `ImagePaste` e `ImageBlend`.
12. Fazer `ImageMatchSubImage` e `ImageLocateSubImage`.
13. Fazer `ImageBlur`.

