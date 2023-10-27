# Trabalho1 AED 2023-2024

Este é o código-fonte para o primeiro projeto de AED 2023-2024.

## Ficheiros

- `image8bit.c` - implementação do módulo
- `image8bit.h` - interface do módulo
- `imageTest.c` - programa de teste simples
- `imageTool.c` - programa de teste mais versátil
- `Makefile`    - regras para compilar e testar usando `make`
- `README.md`   - estas informações que está a ler
- `TIPS.md`     - sugestões para os alunos
- `Design-by-Contract.md` - explicação sobre metodologia DbC


## Compilar

- `make` - Compila e gera os programas de teste.
- `make clean` - Limpa ficheiros objeto e executáveis.



## Atualizar repositório


Dada a natureza do trabalho, poderá ser necessário
atualizar repositório base (upstream) deste projeto.
Se isso acontecer, deverá atualizar o seu repositório com os seguintes comandos:

```bash
git remote add upstream git@github.com:detiuaveiro/image8bit-pub.git
git fetch upstream
git rebase upstream/master
```

## Recursos

Leia as sugestões de desenvolvimento em [`TIPS.md`][tips].

Também pode ler sobre os princípios da [programação por contrato][dbc],
que são seguidos neste projeto.

[tips]: TIPS.md
[dbc]: Design-by-Contract.md


