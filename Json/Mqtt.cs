using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Protocol;
using System;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

public class MQTT
{
    // Variáveis de classe
    private static IMqttClient mqttClient;
    private static string publishTopic = "Senac/Gptrutas/Saida";

    public static async Task Start()
    {
        string broker = "zfeemqtt.eastus.cloudapp.azure.com";
        int port = 41883;
        string clientId = "Senac9";
        string subscribeTopic = "Senac/Gptrutas/Entrada"; // Tópico correto para receber
        string username = "Senac";
        string password = "Senac";

        // Criação do cliente MQTT
        var factory = new MqttFactory();
        mqttClient = factory.CreateMqttClient();

        // Opções do cliente MQTT
        var options = new MqttClientOptionsBuilder()
            .WithTcpServer(broker, port)
            .WithCredentials(username, password)
            .WithClientId(clientId)
            .Build();

        // Conectar ao broker MQTT
        var connectResult = await mqttClient.ConnectAsync(options);

        if (connectResult.ResultCode == MqttClientConnectResultCode.Success)
        {
            Console.WriteLine("Conectado ao broker MQTT");

            // Assinar o tópico de entrada
            await mqttClient.SubscribeAsync(subscribeTopic);
            Console.WriteLine($"Inscrito no tópico: {subscribeTopic}");

            // Função de callback quando uma mensagem é recebida
            mqttClient.ApplicationMessageReceivedAsync += async e =>
            {
                string payload = Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment);
                Console.WriteLine($"Mensagem recebida: {payload}");

                try
                {
                    // Parse da mensagem JSON
                    var jsonDocument = JsonDocument.Parse(payload);
                    if (jsonDocument.RootElement.TryGetProperty("temperature", out JsonElement tempElement))
                    {
                        float temperature = tempElement.GetSingle();

                        // Define o status dependendo da temperatura recebida
                        string status = temperature <= 25.5f ? "Frio" : "Quente";

                        // Cria um JSON de resposta
                        var responseJson = new
                        {
                            status = status
                        };

                        // Serializa o objeto JSON
                        string responseJsonString = JsonSerializer.Serialize(responseJson);

                        // Constrói a mensagem MQTT para o tópico de saída
                        var message = new MqttApplicationMessageBuilder()
                            .WithTopic(publishTopic)  // Tópico de saída
                            .WithPayload(responseJsonString)
                            .WithQualityOfServiceLevel(MqttQualityOfServiceLevel.AtLeastOnce)
                            .Build();

                        // Publica a mensagem no tópico de saída
                        await mqttClient.PublishAsync(message);
                        Console.WriteLine($"Mensagem publicada no tópico {publishTopic}: {responseJsonString}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Erro ao processar a mensagem: {ex.Message}");
                }
            };
        }
        else
        {
            Console.WriteLine($"Falha ao conectar ao broker: {connectResult.ResultCode}");
        }
    }
}
