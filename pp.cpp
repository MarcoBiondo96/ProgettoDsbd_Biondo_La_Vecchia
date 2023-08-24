#include <iostream>
#include <cstring>
#include <conncpp.hpp>
#include <curl\curl.h>
#include <mutex>
#include <unordered_set>
#include "crow.h"
#include <iostream>
#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#define CURL_STATICLIB
using namespace std;
using namespace sql;
using namespace rapidjson;
vector<string> msgs;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {//ww
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string send_post_request(const std::string& url, const std::string& json_data) {
    
    curl_global_init(CURL_GLOBAL_ALL);

    
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return "1";
    }

    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

    
    CURLcode res = curl_easy_perform(curl);

    
    if (res != CURLE_OK) {
        std::cerr << "Failed to send request: " << curl_easy_strerror(res) << std::endl;
    }

    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    return "";
}

void SendPythonRequest(crow::websocket::connection& conn, int x, int y)//ww
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    
    std::string url = "http://localhost:5000/plot?x=" + std::to_string(x) + "&y=" + std::to_string(y);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        
        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        // Invia l'immagine del grafico al client tramite WebSocket
        conn.send_binary(readBuffer);
    }
}
size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data) {//ww
    size_t realsize = size * nmemb;
    data->append(ptr, realsize);
    return realsize;
}

void replaceSpaces(std::string& str) {
    std::string replaceStr = "%20";
    std::string::size_type pos = 0;
    while ((pos = str.find(' ', pos)) != std::string::npos) {
        str.replace(pos, 1, replaceStr);
        pos += replaceStr.length();
    }
}
std::string gamesreturnValue(const std::string& url) {//ww
    
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Imposta l'header della richiesta
        std::string response_str;
        std::ostringstream response_stream;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_str);

        // Esegue la richiesta HTTP
        CURLcode res = curl_easy_perform(curl);

        // Gestisce eventuali errori
        if (res == CURLE_OK) {
            
            return response_str;
        }

        // Pulisce la memoria
        curl_easy_cleanup(curl);
        curl_global_cleanup();

    }
    return "errore";
}


int main()
{

    crow::SimpleApp app;
    std::map<int, crow::websocket::connection*> connections_map;

    CROW_ROUTE(app, "/login")
        .websocket()
        .onopen([](crow::websocket::connection& conn) {
        std::cout << "WebSocket opened" << std::endl;
            })
        .onclose([&connections_map](crow::websocket::connection& conn, const std::string& reason) { //Connection Map implementata inizialmente per effettuare un sistema di notifiche a run time (implementabile in futuro)
                for (auto it = connections_map.begin(); it != connections_map.end(); ++it) {
                    if (it->second == &conn) {
                        connections_map.erase(it);
                        break;
                    }
                }
                
                std::cout << "WebSocket closed: " << reason << std::endl;
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& message, bool is_binary) {
            std::cout << "WebSocket message received: " << message << std::endl;

            Document document;
            document.Parse(message.c_str());

            if (document.HasParseError()) {
                std::cout << "Failed to parse JSON message" << std::endl;
                return "Errore";
            }
            string message_ = document["message"].GetString();
            if (message_ == "login") {
                std::string username = document["username"].GetString();
                std::string password1 = document["password"].GetString();         
                int count = 0;
                try {
                    
                    Driver* driver = mariadb::get_driver_instance();

                    
                    SQLString hostName = "localhost";//Implementare tramite classe static o lettura file ww
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    
                    Statement* stmt = con->createStatement();
                    ResultSet* res = stmt->executeQuery("SELECT *,count(ID) FROM utenti where Username=\"" + username + "\" and Password=\"" + password1 + "\";");

                    Document response;
                    response.SetObject();
                    Value name;
                    
                    res->next();
                    count = res->getInt("Count(ID)");
                    
                    response.AddMember("ID", res->getInt("ID"), response.GetAllocator());
                    int idmapwebsockt = res->getInt("ID");
                    connections_map[idmapwebsockt] = &conn;//implementazione mappa dei websocket

                    response.AddMember("Presente", res->getInt("Count(ID)"), response.GetAllocator());
                    response.AddMember("Nome", name.SetString(res->getString("Nome").c_str(), response.GetAllocator()), response.GetAllocator());
                    response.AddMember("Cognome", name.SetString(res->getString("Cognome").c_str(), response.GetAllocator()), response.GetAllocator());
                    response.AddMember("Username", name.SetString(res->getString("Username").c_str(), response.GetAllocator()), response.GetAllocator());
                        
                    response.AddMember("Email", name.SetString(res->getString("Email").c_str(), response.GetAllocator()), response.GetAllocator());
                    response.AddMember("Data_Nascita", name.SetString(res->getString("Data_Nascita").c_str(), response.GetAllocator()), response.GetAllocator());
                    
                    delete res;
                    delete stmt;
                    delete con;
                    if (count == 1) {
                        
                       
                        Connection* con = driver->connect(hostName, userName, password);//verificare se si può inserire una funzione di connessione ww
                        con->setSchema(dbName);
                        
                        Statement* stmt = con->createStatement();
                        ResultSet* res = stmt->executeQuery("Select Username,Giochi.Nome,Account_Game.Nickname,Account_Game.ID from Utenti Inner Join Account_Game on Utenti.ID=Account_Game.ID_utente INNer Join Giochi On Account_Game.ID_gioco=Giochi.ID where Username=\"" + username + "\";");
                        while (res->next()) {
                            
                            if (res->getString("Nome") == "League of Legends") {
                                
                                response.AddMember("User_lol", name.SetString(res->getString("Nickname").c_str(), response.GetAllocator()), response.GetAllocator());
                            }
                            else if (res->getString("Nome")== "Teamfight Tactics") {
                                
                                response.AddMember("User_tft", name.SetString(res->getString("Nickname").c_str(), response.GetAllocator()), response.GetAllocator());
                            }
                            else if (res->getString("Nome") == "Counter Strike Go") {
                                response.AddMember("User_csgo", name.SetString(res->getString("Nickname").c_str(), response.GetAllocator()), response.GetAllocator());
                            }
                        }
                        delete res;// implementare funzione di canc ww
                        delete stmt;
                        delete con;
                    }
                    
                    StringBuffer buffer;//implementare funzione ww
                    Writer<StringBuffer> writer(buffer);
                    response.Accept(writer);

                    std::string message = buffer.GetString();
                    
                  
                    
                    conn.send_binary(message.data());
                    
                    
                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }

                
            }

            else if (message_ == "visualizza_lobby_id") {
                string id_lobby = document["lobby_id"].GetString();
                try {
                    
                    Driver* driver = mariadb::get_driver_instance();//ww

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    
                    
                    Statement* stmt = con->createStatement();
                    
                    ResultSet* res = stmt->executeQuery("Select utenti.username,Stato_Prenotazione,nota,utenti.id,(account_game.id) AS Account_id from prenotazioni inner join lobby on prenotazioni.ID_lobby=lobby.id inner join account_game on prenotazioni.ID_account=account_game.id inner join utenti on account_game.id_utente=utenti.id where lobby.id=\"" + id_lobby + "\";");
                    Document document;
                    document.SetObject();
                    document.AddMember("type", "stato_utenti_lobby", document.GetAllocator());
                    Value utenti_attesa(kArrayType);
                    Value utenti_accettati(kArrayType);
                    Value names;
                    while (res->next()) {
                        string stato_prenotazione = res->getString("Stato_Prenotazione").c_str();
                        Value utente(kObjectType);
                        if (stato_prenotazione == "Accettato") {
                            
                            utente.AddMember("username", names.SetString(res->getString("username").c_str(), document.GetAllocator()), document.GetAllocator());
                            utente.AddMember("Nota", names.SetString(res->getString("Nota").c_str(), document.GetAllocator()), document.GetAllocator());
                            utente.AddMember("id_utente", res->getInt("id"), document.GetAllocator());
                            utente.AddMember("id_account", res->getInt("Account_id"), document.GetAllocator());
                            utenti_accettati.PushBack(utente, document.GetAllocator());
                        }
                        else {
                            utente.AddMember("username", names.SetString(res->getString("username").c_str(), document.GetAllocator()), document.GetAllocator());
                            utente.AddMember("Nota", names.SetString(res->getString("Nota").c_str(), document.GetAllocator()), document.GetAllocator());
                            utente.AddMember("id_utente", res->getInt("id"), document.GetAllocator());
                            utente.AddMember("id_account", res->getInt("Account_id"), document.GetAllocator());
                            utenti_attesa.PushBack(utente, document.GetAllocator());
                        }
                    }
                    document.AddMember("utenti_accettati", utenti_accettati, document.GetAllocator());
                    document.AddMember("utenti_attesa", utenti_attesa, document.GetAllocator());

                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);

                    string msg = buffer.GetString();
                    
                    delete con;
                    delete stmt;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());
                    

                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "visualizza_utente") {
                string id_utente = document["username"].GetString();
                try {
                    //ww
                    Driver* driver = mariadb::get_driver_instance();

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    

                    Statement* stmt = con->createStatement();

                    ResultSet* res = stmt->executeQuery("select utenti.id,Nickname,Giochi.Nome from utenti inner join account_game on utenti.id=account_game.id_utente inner join Giochi on account_game.id_gioco=giochi.id where utenti.id=\"" + id_utente + "\";");
                    Document document;
                    document.SetObject();
                    Value names;
                    Value value(id_utente.c_str(), document.GetAllocator());
                    document.AddMember("type", "utente_visualizzato", document.GetAllocator());
                    document.AddMember("id_utente", value, document.GetAllocator());
                    
                    while (res->next()) {
                        if (res->getString("Nome") == "League of Legends") {

                            document.AddMember("User_lol", names.SetString(res->getString("Nickname").c_str(), document.GetAllocator()), document.GetAllocator());
                        }
                        else if (res->getString("Nome") == "Teamfight Tactics") {

                            document.AddMember("User_tft", names.SetString(res->getString("Nickname").c_str(), document.GetAllocator()), document.GetAllocator());
                        }
                        else if (res->getString("Nome") == "Counter Strike Go") {
                            document.AddMember("User_csgo", names.SetString(res->getString("Nickname").c_str(), document.GetAllocator()), document.GetAllocator());
                        }
                    }
                    

                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);

                    string msg = buffer.GetString();

                    delete con;
                    delete stmt;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());


                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "partecipa_lobby") {
                string id_lobby = document["lobby_id"].GetString();
                string id_utente = document["id_utente"].GetString();
                string gioco = document["gioco"].GetString();
                string nota= document["nota"].GetString();
                try {
                    
                    Driver* driver = mariadb::get_driver_instance();//ww

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    
                    Statement* stmt = con->createStatement();

                    ResultSet* res = stmt->executeQuery("select account_game.id from utenti inner join account_game on utenti.id=account_game.id_utente inner join giochi on account_game.id_gioco=giochi.id where utenti.id=\"" + id_utente + "\" and giochi.nome=\"" + gioco + "\";");
                    int id_account;
                    res->next();
                    id_account = res->getInt("id");
                    cout << id_account << endl;
                    Statement* stmt2 = con->createStatement();
                    res = stmt2->executeQuery("INSERT INTO Prenotazioni (ID_account, ID_lobby, Nota,Stato_Prenotazione) VALUES (\"" + to_string(id_account) + "\", \"" + id_lobby + "\", \"" + nota + "\", 'Attesa')");


                    Document document;
                    document.SetObject();
                    document.AddMember("type", "partecipazione_inviata", document.GetAllocator());
                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);
                    string msg = buffer.GetString();
                    delete con;
                    delete stmt;
                    delete res;
                    delete stmt2;
                    cout << msg << endl;
                    conn.send_binary(msg.data());


                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "aggiungi_utente") {
                string id_lobby = document["lobby_id"].GetString();
                string id_utente = document["username"].GetString();
                try {
                    //ww
                    Driver* driver = mariadb::get_driver_instance();

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    // Esegui una query AGGIUNGI TRAMITE UN SELECT L'ID ACCOUNT E POI ESEGUIRE LA ELIMINA E LA VISUALIZZA UTENTE 

                    Statement* stmt = con->createStatement();

                    ResultSet* res = stmt->executeQuery("Update prenotazioni set Stato_Prenotazione='Accettato' where id_account=\"" + id_utente + "\" and id_lobby=\"" + id_lobby + "\";");
                    Document document;
                    document.SetObject();
                    document.AddMember("type", "utente_aggiunto", document.GetAllocator());
                    StringBuffer buffer;
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);
                    string msg = buffer.GetString();
                    delete con;
                    delete stmt;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());


                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "elimina_utente") {
                string id_lobby = document["lobby_id"].GetString();
                string id_account = document["username"].GetString();
                try {
                    //ww
                    Driver* driver = mariadb::get_driver_instance();

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    
                    Statement* stmt = con->createStatement();

                    ResultSet* res = stmt->executeQuery("Delete from prenotazioni where id_account=\"" + id_account + "\" and id_lobby=\"" + id_lobby + "\";");
                    Document document;
                    document.SetObject();
                    document.AddMember("type", "utente_eliminato", document.GetAllocator());
                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);
                    string msg = buffer.GetString();
                    delete con;
                    delete stmt;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());


                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "aggiungi_utente") {
                string id_lobby = document["lobby_id"].GetString();
                string id_utente= document["username"].GetString();
                try {
                    //ww
                    Driver* driver = mariadb::get_driver_instance();

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                   
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);



                    Statement* stmt = con->createStatement();

                    ResultSet* res = stmt->executeQuery("Update prenotazioni set Stato_Prenotazione='Accettato' where id_account=\"" + id_utente + "\" and id_lobby=\"" + id_lobby + "\";");
                    Document document;
                    document.SetObject();
                    document.AddMember("type", "utente_aggiunto", document.GetAllocator());
                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);
                    string msg = buffer.GetString();
                    delete con;
                    delete stmt;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());


                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "elimina_lobby") {
                string id_lobby = document["lobby_id"].GetString();
                try {
                    
                    Driver* driver = mariadb::get_driver_instance();//ww

                    
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);


                    

                    Statement* stmt = con->createStatement();

                    ResultSet* res = stmt->executeQuery("Delete from Lobby where lobby.id=\"" + id_lobby + "\";");
                    Document document;
                    document.SetObject();
                    document.AddMember("type", "eliminazione_lobby", document.GetAllocator());
                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);
                    string msg = buffer.GetString();
                    delete con;
                    delete stmt;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());


                }
                catch (SQLException& e) {
                    cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                }
            }
            else if (message_ == "inserisci_lobby") {

                std::string gioco = document["gioco"].GetString();
                std::string nome = document["nome"].GetString();
                std::string id_utente = document["utente_id"].GetString();
                std::string orario = document["orario"].GetString();
                std::string nota = document["nota"].GetString();
                
                
                Driver* driver = mariadb::get_driver_instance();//ww

                
                SQLString hostName = "localhost";
                SQLString userName = "root";
                SQLString password = "marco";
                SQLString dbName = "apl";

                
                Connection* con = driver->connect(hostName, userName, password);
                con->setSchema(dbName);
                try {
                    

                    con->setAutoCommit(false);
                    
                    Statement* stmt = con->createStatement();
                    ResultSet* res = stmt->executeQuery("INSERT INTO Lobby (Orario, Nome, ID_utente) VALUES (\"" + orario + "\", \"" + nome + "\", \"" + id_utente + "\");");
                    Statement* stmt2 = con->createStatement();
                    res = stmt2->executeQuery("SELECT LAST_INSERT_ID()");
                    res->next();
                    int id_lobby = res->getInt(1);
                    Statement* stmt3 = con->createStatement();
                    res = stmt3->executeQuery("Select Account_Game.id from Lobby inner join Utenti on Lobby.ID_utente=Utenti.ID Inner Join Account_Game on Utenti.ID=Account_Game.ID_utente INNer Join Giochi On Account_Game.ID_gioco=Giochi.ID where giochi.nome=\"" + gioco + "\" and utenti.id=\"" + id_utente + "\";");
                    res->next();
                    int id_acc = res->getInt("id");
                    Statement* stmt4 = con->createStatement();
                    stmt4->execute("INSERT INTO Prenotazioni (ID_account, ID_lobby, Nota, Stato_Prenotazione) VALUES (" + std::to_string(id_acc) + ", " + std::to_string(id_lobby) + ", \"" + nota + "\", 'Accettato')");

                    // Conferma la transazione
                    con->commit();

                    std::cout << "Nuova lobby e prenotazione create con successo!" << std::endl;
                    delete con;//ww
                    delete stmt;
                    delete res;
                    delete stmt2;
                    delete stmt3;
                    delete stmt4;
                    
                    string msg = "Lobby Inserita";
                    conn.send_binary(msg.data());
                    
                }
                catch (SQLException& e) {
                    con->rollback();
                    delete con;
                    
                    cout << "Errore  MariaDB: " << e.what() << endl;
                    string msg = "Errore";
                    conn.send_binary(msg.data());
                }
            }

            else if (message_ == "grafico") {
                //chiamata al db 
            }
            else if (message_ == "csstats") {
                
                string nome_ev = document["nome_ev"].GetString();
                string url = "http://localhost:5000/processlogin/" + nome_ev;
                replaceSpaces(url);
                string response = gamesreturnValue(url);
                cout << response << endl;
                
                if (document.HasMember("check")) {

                    if (response != "Errore_account_non_esistente") {

                        try {
                            
                            Driver* driver = mariadb::get_driver_instance();//ww

                            
                            SQLString hostName = "localhost";
                            SQLString userName = "root";
                            SQLString password = "marco";
                            SQLString dbName = "apl";

                            
                            Connection* con = driver->connect(hostName, userName, password);
                            con->setSchema(dbName);


                            
                            string gioco = "Counter Strike Go";
                            string utente = document["Username"].GetString();
                            Statement* stmt = con->createStatement();
                            ResultSet* res = stmt->executeQuery("Select Count(Nickname) from Utenti Inner Join Account_Game on Utenti.ID=Account_Game.ID_utente INNer Join Giochi On Account_Game.ID_gioco=Giochi.ID where Giochi.Nome=\"" + gioco + "\" and Nickname=\"" + nome_ev + "\";");
                            int count = 0;
                            while (res->next()) {
                                count = res->getInt("Count(Nickname)");
                                cout << count << endl;
                            }
                            delete res;
                            delete stmt;
                            delete con;
                            if (count == 0) {
                                Connection* con = driver->connect(hostName, userName, password);
                                con->setSchema(dbName);
                                Statement* stmt = con->createStatement();
                                ResultSet* res = stmt->executeQuery("INSERT INTO Account_Game (ID_utente, ID_gioco, Nickname) SELECT Utenti.ID, Giochi.ID,\"" + nome_ev + "\" FROM Utenti, Giochi WHERE Utenti.Username = \"" + utente + "\" AND Giochi.Nome =\"" + gioco + "\";");
                                delete res;
                                delete stmt;
                                delete con;
                                string msg = "OK";
                                conn.send_binary(msg.data());
                            }
                            else {
                                string msg = "Errore_Account_esistente";
                                conn.send_binary(msg.data());
                            }

                        }
                        catch (SQLException& e) {
                            cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                        }
                    }
                    else
                        conn.send_binary(response.data());
                }
                else
                    conn.send_binary(response.data());
            }
            else if (message_ == "lolstats") {
                string nome_ev = document["nome_ev"].GetString();
                string url = "http://localhost:5000/lol/" + nome_ev;
                replaceSpaces(url);
                string response = gamesreturnValue(url);
                
                if (document.HasMember("check")) {
                    
                    if (response != "Errore_account_non_esistente") {
                        
                        try {
                            //ww
                            Driver* driver = mariadb::get_driver_instance();

                            
                            SQLString hostName = "localhost";
                            SQLString userName = "root";
                            SQLString password = "marco";
                            SQLString dbName = "apl";

                            
                            Connection* con = driver->connect(hostName, userName, password);
                            con->setSchema(dbName);


                            
                            string gioco = "League of Legends";
                            string utente = document["Username"].GetString();
                            Statement* stmt = con->createStatement();
                            ResultSet* res = stmt->executeQuery("Select Count(Nickname) from Utenti Inner Join Account_Game on Utenti.ID=Account_Game.ID_utente INNer Join Giochi On Account_Game.ID_gioco=Giochi.ID where Giochi.Nome=\"" + gioco  + "\" and Nickname=\"" + nome_ev + "\";");
                            int count=0;
                            while (res->next()) {
                                count = res->getInt("Count(Nickname)");
                                cout << count << endl;
                            }
                            delete res;
                            delete stmt;
                            delete con;
                            if (count == 0) {
                                Connection* con = driver->connect(hostName, userName, password);
                                con->setSchema(dbName);
                                Statement* stmt = con->createStatement();
                                ResultSet* res = stmt->executeQuery("INSERT INTO Account_Game (ID_utente, ID_gioco, Nickname) SELECT Utenti.ID, Giochi.ID,\"" + nome_ev + "\" FROM Utenti, Giochi WHERE Utenti.Username = \"" + utente + "\" AND Giochi.Nome =\"" + gioco + "\";");
                                delete res;
                                delete stmt;
                                delete con;
                                string msg = "OK";
                                conn.send_binary(msg.data());
                            }
                            else {
                                string msg = "Errore_Account_esistente";
                                conn.send_binary(msg.data());
                            }

                        }
                        catch (SQLException& e) {
                            cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                        }
                    }
                    else 
                        conn.send_binary(response.data());
                }else
                    conn.send_binary(response.data());
            }
            else if (message_ == "tftstats") {
                
                string nome_ev = document["nome_ev"].GetString();
                string url = "http://localhost:5000/tft/" + nome_ev;
                replaceSpaces(url);
                string response = gamesreturnValue(url);
                cout << response << endl;
                if (document.HasMember("check")) {

                    if (response != "Errore_account_non_esistente") {

                        try {
                            //ww
                            Driver* driver = mariadb::get_driver_instance();

                            
                            SQLString hostName = "localhost";
                            SQLString userName = "root";
                            SQLString password = "marco";
                            SQLString dbName = "apl";

                            
                            Connection* con = driver->connect(hostName, userName, password);
                            con->setSchema(dbName);


                            
                            string gioco = "Teamfight Tactics";
                            string utente = document["Username"].GetString();
                            Statement* stmt = con->createStatement();
                            ResultSet* res = stmt->executeQuery("Select Count(Nickname) from Utenti Inner Join Account_Game on Utenti.ID=Account_Game.ID_utente INNer Join Giochi On Account_Game.ID_gioco=Giochi.ID where Giochi.Nome=\"" + gioco + "\" and Nickname=\"" + nome_ev + "\";");
                            int count = 0;
                            while (res->next()) {
                                count = res->getInt("Count(Nickname)");
                                cout << count << endl;
                            }
                            delete res;
                            delete stmt;
                            delete con;
                            if (count == 0) {
                                Connection* con = driver->connect(hostName, userName, password);
                                con->setSchema(dbName);
                                Statement* stmt = con->createStatement();
                                ResultSet* res = stmt->executeQuery("INSERT INTO Account_Game (ID_utente, ID_gioco, Nickname) SELECT Utenti.ID, Giochi.ID,\"" + nome_ev + "\" FROM Utenti, Giochi WHERE Utenti.Username = \"" + utente + "\" AND Giochi.Nome =\"" + gioco + "\";");
                                delete res;
                                delete stmt;
                                delete con;
                                string msg = "OK";
                                conn.send_binary(msg.data());
                            }
                            else {
                                string msg = "Errore_Account_esistente";
                                conn.send_binary(msg.data());
                            }

                        }
                        catch (SQLException& e) {
                            cout << "Errore di connessione a MariaDB: " << e.what() << endl;
                        }
                    }
                    else
                        conn.send_binary(response.data());
                }
                else
                    conn.send_binary(response.data());
            }
            else if (message_ == "visualizza_lobby") {
                std::string gioco = document["gioco"].GetString();
                std::string user = document["user_id"].GetString();
                try {
                    Driver* driver = mariadb::get_driver_instance();

                    //ww
                    SQLString hostName = "localhost";
                    SQLString userName = "root";
                    SQLString password = "marco";
                    SQLString dbName = "apl";

                    
                    Connection* con = driver->connect(hostName, userName, password);
                    con->setSchema(dbName);

                    Document document;
                    document.SetObject();
                    document.AddMember("type", "visualizza_lobby", document.GetAllocator());
                    Value lobby_possedute(kArrayType);
                    Statement* stmt = con->createStatement();
                    ResultSet* res = stmt->executeQuery("Select Lobby.nome,Lobby.id,Orario,count(Account_Game.id) from Lobby INNER JOIN Prenotazioni On Lobby.ID=Prenotazioni.ID_lobby INNER JOIN Account_Game ON Prenotazioni.ID_account=Account_Game.ID INNER JOIN Giochi ON Account_Game.ID_gioco=Giochi.ID where giochi.nome=\"" + gioco + "\" and lobby.id_utente=\"" + user + "\" and Stato_Prenotazione=\"Accettato\" GROUP BY Lobby.id ;");
                    Value names;
                    while (res->next()) {
                        Value lobby(kObjectType);
                        lobby.AddMember("nomelobby",names.SetString( res->getString("nome").c_str(),document.GetAllocator()), document.GetAllocator());
                        lobby.AddMember("id_lobby", res->getInt("id"), document.GetAllocator());
                        lobby.AddMember("Orario", names.SetString(res->getString("Orario").c_str(),document.GetAllocator()), document.GetAllocator());
                        lobby.AddMember("Accettati", res->getInt("count(Account_Game.id)"), document.GetAllocator());
                        lobby_possedute.PushBack(lobby, document.GetAllocator());
                    }
                    document.AddMember("lobby_possedute", lobby_possedute, document.GetAllocator());

                    Value lobby_presente(kArrayType);
                    Statement* stmt1 = con->createStatement();
                    res = stmt1->executeQuery("Select Lobby.nome,Lobby.id,Orario,count(Account_Game.id) from Lobby INNER JOIN Prenotazioni On Lobby.ID=Prenotazioni.ID_lobby INNER JOIN Account_Game ON Prenotazioni.ID_account=Account_Game.ID INNER JOIN Giochi ON Account_Game.ID_gioco=Giochi.ID where giochi.nome=\"" + gioco + "\" and Stato_Prenotazione=\"Accettato\" and lobby.id_utente!=\"" + user + "\"  AND Lobby.id  IN(SELECT Lobby.id FROM Lobby INNER JOIN Prenotazioni ON Lobby.ID = Prenotazioni.ID_lobby INNER JOIN Account_game on Account_game.id = Prenotazioni.id_account where Account_game.id_utente= \"" + user + "\" and Stato_Prenotazione=\"Accettato\" ) GROUP BY Lobby.id; ");

                    while (res->next()) {
                        Value lobby(kObjectType);
                        lobby.AddMember("nomelobby", names.SetString(res->getString("nome").c_str(), document.GetAllocator()), document.GetAllocator());
                        lobby.AddMember("id_lobby", res->getInt("id"), document.GetAllocator());
                        lobby.AddMember("Orario", names.SetString(res->getString("Orario").c_str(), document.GetAllocator()), document.GetAllocator());
                        lobby.AddMember("Accettati", res->getInt("count(Account_Game.id)"), document.GetAllocator());
                        lobby_presente.PushBack(lobby, document.GetAllocator());
                    }
                    document.AddMember("lobby_presente", lobby_presente, document.GetAllocator());

                    Value lobby_disponibili(kArrayType);
                    Statement* stmt2 = con->createStatement();
                    res = stmt2->executeQuery("Select Lobby.nome,Lobby.id,Orario,count(Account_Game.id) from Lobby INNER JOIN Prenotazioni On Lobby.ID=Prenotazioni.ID_lobby INNER JOIN Account_Game ON Prenotazioni.ID_account=Account_Game.ID INNER JOIN Giochi ON Account_Game.ID_gioco=Giochi.ID where giochi.nome=\"" + gioco + "\" and Stato_Prenotazione=\"Accettato\" and lobby.id_utente!=\"" + user + "\"  AND Lobby.id NOT IN(SELECT Lobby.id FROM Lobby INNER JOIN Prenotazioni ON Lobby.ID = Prenotazioni.ID_lobby INNER JOIN Account_game on Account_game.id = Prenotazioni.id_account where Account_game.id_utente= \"" + user + "\") GROUP BY Lobby.id; "); //Query che preleva il numero di partecipanti accettati ad una lobby e viene visualizzata se l'utente è stato accettato a sua volta dal creatore della lobby


                    while (res->next()) {
                        Value lobby(kObjectType);
                        lobby.AddMember("nomelobby", names.SetString(res->getString("nome").c_str(), document.GetAllocator()), document.GetAllocator());
                        lobby.AddMember("id_lobby", res->getInt("id"), document.GetAllocator());
                        lobby.AddMember("Orario", names.SetString(res->getString("Orario").c_str(), document.GetAllocator()), document.GetAllocator());
                        lobby.AddMember("Accettati", res->getInt("count(Account_Game.id)"), document.GetAllocator());
                        lobby_disponibili.PushBack(lobby, document.GetAllocator());
                    }
                    document.AddMember("lobby_disponibili", lobby_disponibili, document.GetAllocator());
                    
                    
                    StringBuffer buffer;//ww
                    Writer<StringBuffer> writer(buffer);
                    document.Accept(writer);
                    
                    string msg = buffer.GetString();
                    
                    delete con;
                    delete stmt;
                    delete stmt1;
                    delete stmt2;
                    delete res;
                    cout << msg << endl;
                    conn.send_binary(msg.data());
                    
                    

                }
                catch (SQLException& e) {
                    cout << "Errore  " << e.what() << endl;
                }
            }
            
                    });
                    

        CROW_ROUTE(app, "/registrazione")
            .methods(crow::HTTPMethod::Post)([&connections_map](const crow::request& req) {
            
            
        rapidjson::Document doc;
        doc.Parse(req.body.c_str());
        if (doc.HasParseError()) {
            cout << "Failed to parse JSON message" <<endl;
            return "Errore";
        }
        
        string nome = doc["Nome"].GetString();
        
        string cognome = doc["Cognome"].GetString();
        
        string user = doc["Username"].GetString();
        
        string pass = doc["Password"].GetString();
        
        string email = doc["Email"].GetString();
        
        string data_nascita = doc["Data_Nascita"].GetString();
        
        try {
            Driver* driver = mariadb::get_driver_instance();
            SQLString hostName = "localhost";
            SQLString userName = "root";
            SQLString password = "marco";
            SQLString dbName = "apl";
            Connection* con = driver->connect(hostName, userName, password);
            con->setSchema(dbName);
            Statement* stmt = con->createStatement();
            ResultSet* res = stmt->executeQuery("INSERT INTO Utenti (Nome, Cognome, Username, Password, Email, Data_Nascita) VALUES (\"" + nome + "\", \"" + cognome + "\", \"" + user + "\", \"" + pass + "\", \"" + email + "\", \"" + data_nascita + "\")");
            delete res;
            delete stmt;
            delete con;
            auto iter = connections_map.find(1);//implementazione mappe notifiche
            if (iter != connections_map.end()) {
                
                crow::websocket::connection* conns = iter->second;
                string p = "Notifica";
                conns->send_binary(p.data());
            }
            else {
                
                cout << "Errore: websocket non trovato nella mappa." << endl;
            }
            return "OK";
        }
        catch (SQLException& e) {
            cout << "Errore di connessione a MariaDB: " << e.what() << endl;
            return "Errore Username o Email gia' esistenti";
        }
        
        
                });
            app.port(18080).multithreaded().run();
}



