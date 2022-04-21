
// Caméra Wi-Fi ESP32-CAM avec Arduino
// https://tropratik.fr/camera-wi-fi-esp32-cam-avec-arduino/5

// Déclaration des bibliothèques utilisées
#include <WiFi.h>
//#include <ESP32_MailClient.h>
#include <esp_camera.h>
#include <WebServer.h>

// Définition des constantes globales
#define PORT_LED_FLASH      4   // Numéro de port auquel est branchée la LED servant de flash.
 
// Déclaration globales
IPAddress AdresseIpLocale_G; // Permet de mémoriser l'adresse IP de la carte ESP32-CAM
WebServer ServeurFluxVideo_G(80);  // Serveur Web sur le port 80

// Fonction de démarrage, s'exécute une seule fois:
void setup()
{
    // Constantes de la fonction
    const char* SSID_L = "Tenda_8C2A30"; // Nom du réseau Wi-Fi
    const char* MOT_DE_PASSE_L = "existnose620"; // Mot De Passe du réseau

    // Variables de la fonction
    wl_status_t StatutConnexion_L; // Pour mémoriser l'état de la connexion
    esp_err_t   Retour_L;

    // Variables de la fonction
    char Buffer_L[200];

    pinMode(PORT_LED_FLASH, OUTPUT); // Initialisation en "sortie" de la broche d'E/S connectée au flash
    WiFi.begin(SSID_L, MOT_DE_PASSE_L); // Tentative de connexion au point d'accès Wi-Fi
    StatutConnexion_L = WiFi.status(); // Lecture de l'état de la connexion et mémorisation dans la variable "StatutConnexion_L"
    while ((StatutConnexion_L != WL_NO_SSID_AVAIL)&&(StatutConnexion_L != WL_CONNECTED)&&(StatutConnexion_L != WL_CONNECT_FAILED))
    {
        digitalWrite(PORT_LED_FLASH, HIGH);
        delay(100);
        digitalWrite(PORT_LED_FLASH, LOW);
        delay(500);
        StatutConnexion_L = WiFi.status(); // Lecture de l'état de la connexion et mémorisation dans la variable "StatutConnexion_L"
    }

    Serial.begin(115200); // Ouverture du port série à 115200 bauds

    // Affichage du résultat de la tentative de connexion
    if (StatutConnexion_L == WL_CONNECTED)
    {
        Serial.println("Connection OK");
        AdresseIpLocale_G = WiFi.localIP(); // Mémorisation de l'adresse actuelle
        Retour_L = InitialiserCamera();
        if(Retour_L == ESP_OK)
        {
          sprintf(Buffer_L,"<p><strong>La caméra a démarré avec succès !</strong></p><p>Cliquez sur le lien \"http://%u.%u.%u.%u\" pour vous connecter.</p>", AdresseIpLocale_G[0], AdresseIpLocale_G[1], AdresseIpLocale_G[2], AdresseIpLocale_G[3]);
        }
        else
        {
          sprintf(Buffer_L,"<p><strong>Erreur d'initialisation de la caméra !</strong></p><p>L'erreur 0x%x a été rencontrée.</p>", Retour_L);
        }
        //Serial.println("Démarrage de la caméra Wi-Fi", Buffer_L);
        //EnvoyerEmail("Démarrage de la caméra Wi-Fi", Buffer_L);
        ServeurFluxVideo_G.on("/", HTTP_GET, FluxVideo); // La racine du site Web est associée à la fonction "FluxVideo"
        ServeurFluxVideo_G.begin(); // Démarrage du serveur
    }
    else if (StatutConnexion_L == WL_NO_SSID_AVAIL)
    {
        Serial.println("SSID introuvable");
    }
    else if (StatutConnexion_L == WL_CONNECT_FAILED)
    {
        Serial.println("Mot de passe KO");
    }
    else
    {
        Serial.println("Autre erreur");
    }
}

// Fonction principale du programme, s'exécute en boucle:
void loop()
{
    // Variables de la fonction
    char Buffer_L[200];

    if(WiFi.status()==WL_CONNECTED)
    {  
        // Internet est disponible
        if (WiFi.localIP()!=AdresseIpLocale_G)
        {
            // L'ESP32-CAM vient d'obtenir une nouvelle adresse IP
            AdresseIpLocale_G = WiFi.localIP(); // Mémorisation de l'adresse actuelle
            sprintf(Buffer_L,"<p><strong>La caméra a changé d'adresse IP</strong></p><p>Cliquez sur le lien \"http://%u.%u.%u.%u\" pour vous connecter.</p>", AdresseIpLocale_G[0], AdresseIpLocale_G[1], AdresseIpLocale_G[2], AdresseIpLocale_G[3]);
            // EnvoyerEmail("Changement d'adresse IP de la caméra Wi-Fi", Buffer_L);
        }
        ServeurFluxVideo_G.handleClient(); // Traitement des requêtes des clients du serveur Web
    }
    else
    {
        // Pas de connexion internet
        digitalWrite(PORT_LED_FLASH, HIGH);
        delay(100);
        digitalWrite(PORT_LED_FLASH, LOW);
        delay(500);
    }
}
/*
void EnvoyerEmail(const char *pObjet_P, const char *pMessage_P)
{
    const char* ADRESSE_EMAIL_EMISSION_L = "votremaildetest@gmail.com";
    const char* MOT_DE_PASSE_EMAIL_EMISSION_L = "12345MoteDePasseMail";
    const char* ADRESSE_SERVEUR_SMTP_L = "smtp.gmail.com";
    int PORT_SERVEUR_SMTP_L = 465; // Port SSL utilisé
    const char* ADRESSE_EMAIL_DESTINATAIRE_L = "votremail@gmail.com";

    // Variables locales de la fonction
    SMTPData DonneesEmail_L;  // Données de l'e-mail à envoyer
    
    // Les caractéristique de l'e-mail sont renseignées
    DonneesEmail_L.setLogin(ADRESSE_SERVEUR_SMTP_L, PORT_SERVEUR_SMTP_L, ADRESSE_EMAIL_EMISSION_L, MOT_DE_PASSE_EMAIL_EMISSION_L); // Identifiants de connexion au serveur SMTP
    DonneesEmail_L.setSender("Camera Wi-Fi", ADRESSE_EMAIL_EMISSION_L); // Nom et adresse email de l'emetteur
    DonneesEmail_L.setPriority("Normal"); // Niveau d'importance du mail
    DonneesEmail_L.setSubject(pObjet_P);  // Objet du mail
    DonneesEmail_L.setMessage(pMessage_P, true); // Contenu du message au format HTML
    DonneesEmail_L.addRecipient(ADRESSE_EMAIL_DESTINATAIRE_L);  // Adresse du destinataire
    
    //Emission de l'e-mail
    if (!MailClient.sendMail(DonneesEmail_L))
    {
      Serial.println("Erreur lors de l'envoi d'e-mail:" + MailClient.smtpErrorReason());
    }
    DonneesEmail_L.empty(); // Nettoyage des données d'envoi d'e-mail
}
*/
esp_err_t InitialiserCamera()
{
    // Variables locales de la fonction
    esp_err_t       Retour_L;
    camera_config_t ConfigurationCamera_L;

    // Cablage de la caméra sur l'ESP32-CAM du fabricant AI-Thinker
    ConfigurationCamera_L.pin_d0 = 5;
    ConfigurationCamera_L.pin_d1 = 18;
    ConfigurationCamera_L.pin_d2 = 19;
    ConfigurationCamera_L.pin_d3 = 21;
    ConfigurationCamera_L.pin_d4 = 36;
    ConfigurationCamera_L.pin_d5 = 39;
    ConfigurationCamera_L.pin_d6 = 34;
    ConfigurationCamera_L.pin_d7 = 35;
    ConfigurationCamera_L.pin_xclk = 0;
    ConfigurationCamera_L.pin_pclk = 22;
    ConfigurationCamera_L.pin_vsync = 25;
    ConfigurationCamera_L.pin_href = 23;
    ConfigurationCamera_L.pin_sscb_sda = 26;
    ConfigurationCamera_L.pin_sscb_scl = 27;
    ConfigurationCamera_L.pin_pwdn = 32;
    ConfigurationCamera_L.pin_reset = -1;

    // La génération du signal d'horloge
    ConfigurationCamera_L.ledc_channel = LEDC_CHANNEL_0;
    ConfigurationCamera_L.ledc_timer = LEDC_TIMER_0;
    ConfigurationCamera_L.xclk_freq_hz = 20000000;

    // Compression jpeg
    ConfigurationCamera_L.pixel_format = PIXFORMAT_JPEG;
    ConfigurationCamera_L.fb_count = 2;
    ConfigurationCamera_L.jpeg_quality = 10;

    // Résolution de l'image
    ConfigurationCamera_L.frame_size = FRAMESIZE_SVGA;

    // Lancement de l'initialisation de la caméra
    Retour_L=esp_camera_init(&ConfigurationCamera_L);
    if (Retour_L == ESP_OK)
    {
        Serial.printf("La camera est initialisee\n");
    }
    else
    {
        Serial.printf("Erreur 0x%x lors de l'initialisation de la camera\n", Retour_L);
    }
    return(Retour_L);
}

void FluxVideo()
{
    // Variables locales de la fonction
    String EnteteHtmlInitiale_L = "HTTP/1.1 200 OK\r\n" \
        "Access-Control-Allow-Origin: *\r\n" \
        "Content-Type: multipart/x-mixed-replace; boundary=FinDeLaSection\r\n\r\n;";
    String EnteteHtmlIntermediaire_L = "--FinDeLaSection\r\n" \
        "Content-Type: image/jpeg\r\n\r\n";
    WiFiClient Client_L;
    camera_fb_t *TamponVideo_L = NULL; 
  
    ServeurFluxVideo_G.sendContent(EnteteHtmlInitiale_L);
    Client_L = ServeurFluxVideo_G.client();
    while (Client_L.connected())
    {   
        // Le navigateur Web client est toujours connecté
        TamponVideo_L = esp_camera_fb_get(); // Récupère une image depuis la caméra      
        ServeurFluxVideo_G.sendContent(EnteteHtmlIntermediaire_L);
        Client_L.write((char *)TamponVideo_L->buf, TamponVideo_L->len);
        ServeurFluxVideo_G.sendContent("\r\n");
         
        // Réamorcage du tampon video, indispensable pour sa réutilisation
        if (TamponVideo_L)
        {
            esp_camera_fb_return(TamponVideo_L);
            TamponVideo_L = NULL;
        }
    }
}
