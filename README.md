# Sistema IoT com Processamento em Borda para Gestão Inteligente de Água em Estufas Agrícolas

Este projeto consiste no desenvolvimento de um sistema de Internet das Coisas
(IoT) baseado em um microcontrolador ESP32 para gestão de recursos hídricos em
estufas agrícolas. Ele monitora continuamente variáveis ambientais (temperatura
e umidade) e o nível de um reservatório, executando o controle automático de uma
bomba d’água por meio de relé para manter o nível da solução nutritiva constante
(ligando em < 50% e desligando em > 99%). Os dados são transmitidos de forma
segura para plataformas de nuvem, onde é possível visualizar o status em tempo
real e enviar comandos remotos de interrupção de emergência via MQTT.


## Plataformas Necessárias

**Atenção:** Este projeto depende de duas plataformas de nuvem externas para
monitoramento e visualização completa dos dados:

*   **Ubidots** - para dashboard em tempo real e recebimento de comandos remotos
*   **ThingSpeak** - para análise estatística e armazenamento de dados
    históricos


## Como Rodar no Wokwi

1.  Acesse o link do projeto no Wokwi.
2.  A simulação carregará automaticamente o ESP32 e todos os componentes
    (sensores, relé e LED).
4.  Clique no botão **"Run"** (o triângulo verde) no simulador. O
    microcontrolador iniciará a conexão Wi-Fi e o envio periódico de dados para
    as nuvens configuradas.


## Links do Projeto

*   **Simulação (WokWi):** <https://wokwi.com/projects/455672056860199937>
*   **Dashboard Ubidots:** <https://industrial.ubidots.com/app/dashboards/public/dashboard/ZgCbYRmr8VX2Gt4HXlN8uxJxdCj0OZsA>
*   **Canal ThingSpeak** (requer login): <https://thingspeak.mathworks.com/channels/3258510/shared_view>

