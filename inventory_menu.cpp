#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

// inventory item
struct InventoryItem {
    int id;
    string name;
    int quantity;
    double price;
};

vector<InventoryItem> inventory;
int nextId = 1;

// write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// deepSeek api
string callDeepSeekAPI(const string& prompt) {
    CURL* curl;
    CURLcode res;
    string readBuffer;
    string apiKey = "api-key"; // Your API key
    string url = "https://api.deepseek.com/chat/completions";

    curl = curl_easy_init();
    if (curl) {
        // json data
        Json::Value root;
        root["model"] = "deepseek-chat";
        
        Json::Value messages(Json::arrayValue);
        
        Json::Value systemMsg;
        systemMsg["role"] = "system";
        systemMsg["content"] = "You are an expert inventory management assistant. Provide helpful, concise advice about inventory management, product ordering, and stock control.";
        messages.append(systemMsg);
        
        Json::Value userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = prompt;
        messages.append(userMsg);
        
        root["messages"] = messages;
        root["stream"] = false;
        
        Json::StreamWriterBuilder writer;
        string jsonData = Json::writeString(writer, root);
        
        // Set headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Perform the request
        res = curl_easy_perform(curl);
        
        // Check for errors
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "Error: API request failed.";
        }
        
        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        
        // Parse the JSON response
        Json::CharReaderBuilder readerBuilder;
        Json::Value responseJson;
        string errors;
        istringstream responseStream(readBuffer);
        
        if (Json::parseFromStream(readerBuilder, responseStream, &responseJson, &errors)) {
            if (responseJson.isMember("choices") && responseJson["choices"].isArray() && 
                responseJson["choices"].size() > 0) {
                Json::Value message = responseJson["choices"][0]["message"];
                if (message.isMember("content")) {
                    return message["content"].asString();
                }
            }
            return "Error: Invalid response format from API.";
        } else {
            return "Error: Failed to parse API response.";
        }
    }
    return "Error: Could not initialize CURL.";
}

// Function to add a new inventory item
void addItem() {
    InventoryItem newItem;
    newItem.id = nextId++;
    
    cout << "Enter item name: ";
    cin.ignore();
    getline(cin, newItem.name);
    
    cout << "Enter quantity: ";
    cin >> newItem.quantity;
    
    cout << "Enter price: ";
    cin >> newItem.price;
    
    inventory.push_back(newItem);
    cout << "Item added successfully!" << endl;
}

// Function to display all inventory items
void displayInventory() {
    if (inventory.empty()) {
        cout << "Inventory is empty." << endl;
        return;
    }
    
    cout << "\nID\tName\tQuantity\tPrice" << endl;
    cout << "----------------------------------------" << endl;
    for (const auto& item : inventory) {
        cout << item.id << "\t" << item.name << "\t" << item.quantity << "\t\t$" << item.price << endl;
    }
    cout << endl;
}

// Function to update an inventory item
void updateItem() {
    int id;
    cout << "Enter the ID of the item to update: ";
    cin >> id;
    
    for (auto& item : inventory) {
        if (item.id == id) {
            cout << "Enter new quantity: ";
            cin >> item.quantity;
            
            cout << "Enter new price: ";
            cin >> item.price;
            
            cout << "Item updated successfully!" << endl;
            return;
        }
    }
    
    cout << "Item with ID " << id << " not found." << endl;
}

// Function to delete an inventory item
void deleteItem() {
    int id;
    cout << "Enter the ID of the item to delete: ";
    cin >> id;
    
    for (auto it = inventory.begin(); it != inventory.end(); ++it) {
        if (it->id == id) {
            inventory.erase(it);
            cout << "Item deleted successfully!" << endl;
            return;
        }
    }
    
    cout << "Item with ID " << id << " not found." << endl;
}

// Function to interact with DeepSeek AI
void askAI() {
    cin.ignore(); // Clear input buffer
    
    string prompt;
    cout << "Ask the AI assistant about inventory management: ";
    getline(cin, prompt);
    
    cout << "\nAI is thinking...\n" << endl;
    
    string response = callDeepSeekAPI(prompt);
    cout << "AI Assistant: " << response << endl;
}

// Main function
int main() {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    int choice;
    
    cout << "==========================================" << endl;
    cout << "     INVENTORY MANAGEMENT SYSTEM" << endl;
    cout << "     with DeepSeek AI Integration" << endl;
    cout << "==========================================" << endl;
    
    do {
        cout << "\nMenu Options:" << endl;
        cout << "1. Add Item" << endl;
        cout << "2. Display Inventory" << endl;
        cout << "3. Update Item" << endl;
        cout << "4. Delete Item" << endl;
        cout << "5. Ask AI Assistant" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                addItem();
                break;
            case 2:
                displayInventory();
                break;
            case 3:
                updateItem();
                break;
            case 4:
                deleteItem();
                break;
            case 5:
                askAI();
                break;
            case 6:
                cout << "Exiting program. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 6);
    
    // Clean up CURL
    curl_global_cleanup();
    
    return 0;
}

