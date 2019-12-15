# Projeto de Rastreamento Planar
Segundo projeto da disciplina Processamento Gráfico(IF680) feito em dupla por Igor Beltrão Pereira Simões(ibps) e Victor Edmond Freire Gaudiot(vefg).

### Objetivo
Dada uma imagem de refência, identifica-lá no vídeo dado (tanto da webcam quanto vídeos normais) e destacá-la com um contorno colorido.

### Funcionamento
Optamos por utilizar o SIFT como algoritmo padrão para detectar os pontos chaves e descritores da imagem/frame, mas também temos SURF e AKAZE especificados no código, basta retirar comentários como explicado no próprio código.

Para a realização do casamento de pontos entre os pontos chaves da imagem de referência e da imagem do vídeo, utilizamos a biblioteca FLANN, onde comparavamos pontos chaves 2 a 2 e escolhíamos pontos que se encaixavam na proporção de distância de Lowe.

#### Como executar:
Esse repositório contém apenas o main.cpp, já que a estrutura para compilar e exectuar o projeto possui arquivos grandes. Portanto, deve-se baixar o zip por meio do link abaixo e extraí-lo localmente. Após isso, faz-se necessário substituir o main.cpp presente na pasta "Projeto-PG" pelo main.cpp desse repositório e apenas então utilizar o Visual Studio para abrir o projeto pelo arquivo "Projeto-PG.sln".

* [Baixar Zip](https://drive.google.com/open?id=1KZh4tXiT8_AgP5DG7DzmcRgfQv28MDIc) - Link para download do zip com a lib buildada e compilada.
* [Instruções](https://drive.google.com/open?id=1iBeXNe9_XvyT2smlTcSJb8ub0pnBHZ1L) - Caso você esteja com problemas para rodar o projeto no Visual Studio, tente as soluções nesse link.

A lib foi buildada e compilada pelo monitor da disciplina Josue Fidelis de Oliveira Neto.
