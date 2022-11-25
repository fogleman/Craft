## Craft

Clone de Minecraft para Windows, Mac OS X e Linux. Usando somente algumas linhas de C com o moderno OpenGL (shaders). O suporte a multiplayer online esta incluido usando um server baseado em Python.

http://www.michaelfogleman.com/craft/

![Screenshot](https://i.imgur.com/SH7wcas.png)

### Características

* Simples porem um bom gerador de terreno usando perlin / simplex noise.
* Mais de 10 tipos de blocos com a possibilidade de mais serem adicionados de forma simples.
* Suporte a plantas (grama, flores, arvores, etc.) e transparência (Vidro).
* Nuvens simples no céu (elas não se movem).
* Ciclo de dia / noite e a textura de sky dome.
* Mudanças de mundo baseadas em uma sqlite3 database.
* Suporte a multiplayer!

### Download

Mac and Windows binaries Disponiveis no site.

http://www.michaelfogleman.com/craft/

Veja abaixo a origem.

### Instalar dependências

#### Mac OS X

Baixe e instale [CMake](http://www.cmake.org/cmake/resources/software.html)
caso você não tenha. você pode usar [Homebrew](http://brew.sh) para simplificar
a instalação:

    brew install cmake

#### Linux (Ubuntu)

    sudo apt-get install cmake libglew-dev xorg-dev libcurl4-openssl-dev
    sudo apt-get build-dep glfw

#### Windows

Baixe e instale [CMake](http://www.cmake.org/cmake/resources/software.html)
e [MinGW](http://www.mingw.org/). e `C:\MinGW\bin` ao seu `PATH`.

Baixe e instale [cURL](http://curl.haxx.se/download.html) com
CURL/lib e CURL/include no seu diretorio de programas e arquivos.

Use os seguintes comandos no lugar desses descritos na proxima sessão.

    cmake -G "MinGW Makefiles"
    mingw32-make

### Compile e incie

Uma vez que você tenha os requisitos (veja acima), incie os seguintes comandos no seu
terminal.

    git clone https://github.com/fogleman/Craft.git
    cd Craft
    cmake .
    make
    ./craft

### Multiplayer

Após muitos anos, craft.michaelfogleman.com foi tirado do ar. veja a sessão [Server](#server) para informações de host próprio.

#### Client

Você pode conectar a um servidor usando as seguintes linhas de codigós...

```bash
./craft [HOST [PORT]]
```

Ou, com o comando "/online" no jogo em si.
    
    /online [HOST [PORT]]

#### Servidor

Você pode iniciar seu próprio servidor ou conectar ao meu. O servidor é escrito em Python
mas requere um compilador DLL para a geração de terreno performar igual 
a do client.

```bash
gcc -std=c99 -O3 -fPIC -shared -o world -I src -I deps/noise deps/noise/noise.c src/world.c
python server.py [HOST [PORT]]
```

### Controles

- WASD para mover em frente, esquerda, trás, direita.
- Barra de espaço para pular.
- Esquerdo do mouse para distruir um bloco.
- Direito do mouse ou Cmd + Esquerdo do mouse para criar um bloco.
- Ctrl + Direito do mouse to alternar um bloco como na source original.
- 1-9 para selecionar o tipo de bloco que deseja criar.
- E Visualisar os tipos de blocos disponíveis.
- Tab para mudar entre andar e voar.
- ZXCVBN para se mover ao eixos de  XYZ.
- Shift esquerdo para um zoom.
- F mostrar a cena em uma forma organica.
- O Para observar um player em primeira pessoa.
- P para observar um player de forma picture-in-picture.
- T para digitar no chat.
- Barra (/) para digitar um comando.
- Aspas simples (`) para escrever um texto em qualquer bloco (placas).
- Setas para simular um movimento de mouse.
- Enter simular um clique de mouse.

### Comandos de chat

    /goto [NAME]

Teleportar a algum player.
Caso não seja expecificado um nome, um player aleatório sera escolhido.

    /list

Mostrar a lista de players conectados.

    /login NAME

Mudar para algum usuario ja cadastrado.
Será feito um novo login no server.

    /logout

Sair de um usuario e se tornar um usuario desconhecido.
Logins automaticos não devem acontecer enquando o comando /login esta sendo re-feito.

    /offline [FILE]

Mudar para o modo offline.
FILE especifica o lugar de salvamento, para usar o padrão use o "craft".

    /online HOST [PORT]

Conectar a um servidor específico.

    /pq P Q

Teleporar para um chunk específico.

    /spawn

Teleportar ao local de spawn.

### Screenshot

![Screenshot](https://i.imgur.com/foYz3aN.png)

### Detalhes implementados

#### Geração de terrenpo

O terreno é gerado usando a Simplex noise - uma função de noise determinística baseada em posição. Asiim o mundo sempre será gerado da mesma forma de a acordo com a localização recebida.

O mundo é dividido em um chunk de 32x32 blocos no plano XZ (Y sendo a altura). Isso permite o mundo ser “infinito” (A precisão dos pontos flutuantes esta sendo um problema em grande valores de X ou Z ) e também permite que seja mais facil manusear os dados. Somentes os chunks visíveis são usados do banco de dados.

#### Renderização

Somente as faces expostas são rederizadas. Isso é uma importante forma de otimização ja que a vasta maioria dos blocos tem faces que não estão expostas. Cada chunk faz uma copia de um chunk vizinho isso significa que o sistema vai saber quais faces de blocos estão expostas.

Somentes os chunks visiveis são renderizados. A simples frustum-culling approach é usado para teste se a chunck esta na visão da camera. E se não esta, ela não é renderizada. Isso caba resultando num ganho decente de performance.

Chunk buffers são completamente refeitos quando existe uma mudança em um bloco na mesma chunk, ao invés de tentar atualizar usando o VBO.

O texto é renderizado usando um atlas bitmap. onde cada caractere é renderizado em dois triangulos formando um retangulo 2d.

“Moderno” OpenGL é usado - não obsoleto, funções corrigidas agora a função de gasoduto é usado . Os objetos do Vertex buffer são usados para a posição, normal e texturas cordenadas. Vertex e fragmentos de Shaders são usados para renderização. Manipulações na Matrix são feitas na  matrix.c para transações, rotações, perspectiva, ortografia, etc. matrizes. Os modelos 3d são feitos de uma perpectiva bem simples - a maioria sendo cubos ou retangulos. Esses modelos são gerados em codigos na cube.c.

Transparencia nos blocos de vidro e plantas (plantas não possuem um formato 100% retangular das suas formas triangulares primitvas) é implementado descartando pixels de cor magenta nos fragment shader.

#### Banco de dados

As mudanças que o usuario faz no mundo são armazenadas no banco de dados sqlite. Somente o delta é armazenado, assim o mundo padrão é gerado e as mundanças que o usuario fez são aplicados em cima durante o carregamento.

O nome do principal nome de dados é “block” e tem colunas p, q, x, y, z, w. (p, q) identificam a chunk, (x, y, z) identificam a posição dos blocos e (w) identifica o tipo de bloco. 0 representa um bloco vazia (ar).

Em jogo, os chunks armazenam seus blocos em um mapa de hash. Com (x, y, z) como chaves de mapa (w) sendo o valor.

A posição de y dos blocos são limitados de 0 < = y < 256. O limite de altura é uma limitação aritifical para prevenir os usuarios de construir estruturas desnecessariamente altas. Os usuarios não são permitidos de quebrar blocos na posição y = 0 para evitar de cair no vazio interminavel do mundo.

#### Multiplayer

O modo Multiplayer é implementado usando sockets antigos. Um simples, ASCII, um protocolo baseado em linha é usado. Cada linha é feita usando um codigo de comando e zero ou mais  argumentos. O cliente requisita chunks do server com um simples comandos: C,p,q,key. “C” significando “Chunk” e (p, q) identificando a chunk. A chave é usada para cache - o servidor enviará apenas atualizações de bloco que foram executadas desde a última vez que o cliente solicitou esse bloco. Atualizações de blocos (em tempo real ou na parte em que foi requisitada pela chunk) são enviados ao cliente no formato: B,p,q,x,y,z,w. Depois de enviar todos os blocos para um pedido a chunk, o servidor enviará uma chave de cache atualizada no formato: K,p,q,key. O cliente armazenará essa chave e a usará na próxima vez que precisar solicitar a essa chunck. As posições dos jogadores são enviadas no formato: P,pid,x,y,z,rx,ry. O pid é o ID do player e os valores de rx e ry são indicados através da rotação do eixo do player. O cliente interpola as posições dos players das últimas duas atualizações de posição para uma animação mais suave. O cliente envia a posição para o servidor no máximo a cada 0,1 segundos (menos se não estiver em movimento).
 (less if not moving).


Cache do lado do cliente para o banco de dados sqlite pode ser intensivo em desempenho ao se conectar a um servidor pela primeira vez. Por essa razão, as escritas de sqlite performão no plano de fundo da thread. Todas as gravações ocorrem em uma transação para desempenho. A transação é confirmada a cada 5 segundos em oposição a alguma quantidade lógica de trabalho concluído. Um buffer de anel/circular é usado como uma fila para quais dados devem ser gravados no banco de dados.

No modo multiplayer, players podem observar uns aos outros na visão principal ou em uma visão picture-in-picture.A implementação do PnP foi surpreendentemente simples - apenas mude a janela de visualização e renderize a cena novamente do ponto de vista do outro Player.

#### Teste de colisão

Teste de golpe (para qual bloco o usuário está apontando) é implementado pela varredura de um raio da posição do jogador para fora, seguindo seu vetor de visão. Este não é um método preciso, então a taxa de passo pode ser menor para ser mais preciso.

O teste de colisão simplesmente ajusta a posição do player para permanecer a uma certa distância de quaisquer blocos adjacentes que sejam obstáculos. (Nuvens e plantas não são marcadas como obstáculos, então você passa direto por elas.)

#### Sky Dome

A textura do sky dome é usada para o céu. A coordenada X representa a hora do dia. Os valores de Y mapeiam da parte inferior da esfera do céu até o topo da esfera do céu. O jogador está sempre no centro da esfera. Os shaders de fragmentos para os blocos também amostram a textura do céu para determinar a cor de névoa apropriada para misturar com base na posição do bloco em relação ao céu de fundo.

#### Oclusão de Ambiente

A oclusão de ambiente é implementada conforme descrito nesta página:

http://0fps.wordpress.com/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/

#### Dependencies

* GLEW é usado para gerenciar as extensões do OpenGL entre as plataformas.
* GLFW é usado para gerenciamento de janela entre plataformas.
* CURL é usado para HTTPS / SSL POST para o processo de autenticação.
* lodepng é usado para carregar texturas PNG.
* sqlite3 é usado para salvar blocos adiconados / removidos pelo usario.
* tinycthread é usado para o threading entre plataformas.
