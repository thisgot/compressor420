INTRO:

o programa:
*   le uma imagem (image.png) no mesmo diretorio (a imagem deve possuir dimensoes pares)
*   comprime a imagem para o formato 4:2:0 (reduz pela metade o tamanho)
*   descomprime para o formato RGB novamente
*   salva a imagem como (image_compressed_n_decompressed.png)

nao ha mudanca entre o tamanho das final das duas imagens, a reducao ocorre dentro do programa,
o motivo de se salvar a imagem eh para comparacao da qualidade da imagem que sofreu compressão e a orignal

COMO RODAR:

*   python: no terminal entre 'py compress420.py'
*   c++: no terminal entre './compressor420.exe'