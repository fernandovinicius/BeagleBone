Descrição
--------------------------------------------------------------------------------------
Exemplo de como controlar o conversor AD interno da BBB através da PRU.

Realiza a leitura de um canal do conversor, a uma determinada taxa de amostragem, 
durante um período de tempo especificado, e salva os dados da leitura em um arquivo txt.

Compilar
--------------------------------------------------------------------------------------

    $ make clean; make

Executar
--------------------------------------------------------------------------------------

    # ./host_main <CHANNEL> <SAMPLE_RATE_HZ> <DURATION_SEC>

Exemplo
--------------------------------------------------------------------------------------

Canal: 0; Taxa de amostragem: 1000 Hz; Duração: 10 segundos

    # ./host_main 0 1000 10

Parâmetros Aceitos
--------------------------------------------------------------------------------------
  - Canais: 0-6
  - Taxas de amostragem (Hz): 1600000,  800000, 400000, 200000, 100000, 50000, 20000, 10000, 5000, 2000, 1000, 500, 200, 100

Configurações
--------------------------------------------------------------------------------------
Esse programa foi desenvolvido e testado com as seguintes configurações:
 - Placa BeagleBone Black Rev C
 - Debian Strech 4.9.51-bone7 Debian Image 2017-09-17
 - Compilador GCC 6.3.0 20170516 (Debian 6.3.0-18)
 - Assembler PASM Version 0.87
 
Observações gerais
--------------------------------------------------------------------------------------

Verificar se o módulo "uio_pruss" está carregado no sistema:

    $ lsmod | grep uio_pruss

Caso não esteja, abrir o arquivo '/boot/uEnv.txt' como root e, em seguida, procurar e comentar as seguintes linhas (inserir # no começo da linha):

    uboot_overlay_pru=/lib/firmware/AM335X-PRU-RPROC-4-4-TI-00A0.dtbo
    enable_uboot_cape_universal=1

E descomentar a seguinte linha (remover, se houver, o # do começo da linha):

    uboot_overlay_pru=/lib/firmware/AM335X-PRU-UIO-00A0.dtbo

Salve o arquivo, e reinicie o sistema.

Pool RAM
--------------------------------------------------------------------------------------
Verificar endereço e tamanho da memória alocada (Pool RAM) para transferência de dados entre a PRU e Linux:

    $ cat /sys/devices/platform/ocp/4a300000.pruss/uio/uio0/maps/map1/addr
    $ cat /sys/devices/platform/ocp/4a300000.pruss/uio/uio0/maps/map1/size
   
O tamanho desse espaço de memória tem relação direta com a quantidade máxima de dados a serem coletados.
Caso deseje aumentar esse espaço:

    # sudo rmmod uio_pruss
    # sudo modprobe uio_pruss extram_pool_sz=<SIZE_HEX>

 * <SIZE_HEX> = Tamanho da memória em bytes, expresso em hexadecimal.

Exemplo: (Alocar 2 Milhões de bytes)

    # sudo rmmod uio_pruss
    # sudo modprobe uio_pruss extram_pool_sz=0x1E8480

