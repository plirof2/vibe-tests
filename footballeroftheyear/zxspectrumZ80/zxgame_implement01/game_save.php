<?php
// game_save.php - PHP backend for game save/load functionality

class GameSaveManager {
    private $db;
    
    public function __construct() {
        $this->db = new PDO('mysql:host=localhost;dbname=football_game', 'username', 'password');
        $this->db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $this->createTables();
    }
    
    private function createTables() {
        $this->db->exec("
            CREATE TABLE IF NOT EXISTS game_saves (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL,
                save_name VARCHAR(100) NOT NULL,
                game_data TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
            )
        ");
        
        $this->db->exec("
            CREATE TABLE IF NOT EXISTS users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) UNIQUE NOT NULL,
                password VARCHAR(255) NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ");
    }
    
    public function saveGame($userId, $saveName, $gameData) {
        $stmt = $this->db->prepare("
            INSERT INTO game_saves (user_id, save_name, game_data) 
            VALUES (:user_id, :save_name, :game_data)
        ");
        $stmt->execute([
            ':user_id' => $userId,
            ':save_name' => $saveName,
            ':game_data' => json_encode($gameData)
        ]);
        return $this->db->lastInsertId();
    }
    
    public function loadGame($saveId) {
        $stmt = $this->db->prepare("
            SELECT game_data FROM game_saves WHERE id = :id
        ");
        $stmt->execute([':id' => $saveId]);
        $result = $stmt->fetch(PDO::FETCH_ASSOC);
        return $result ? json_decode($result['game_data'], true) : null;
    }
    
    public function getUserSaves($userId) {
        $stmt = $this->db->prepare("
            SELECT id, save_name, created_at, updated_at 
            FROM game_saves 
            WHERE user_id = :user_id 
            ORDER BY updated_at DESC
        ");
        $stmt->execute([':user_id' => $userId]);
        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    }
    
    public function createUser($username, $password) {
        $hashedPassword = password_hash($password, PASSWORD_DEFAULT);
        $stmt = $this->db->prepare("
            INSERT INTO users (username, password) 
            VALUES (:username, :password)
        ");
        $stmt->execute([
            ':username' => $username,
            ':password' => $hashedPassword
        ]);
        return $this->db->lastInsertId();
    }
    
    public function authenticateUser($username, $password) {
        $stmt = $this->db->prepare("
            SELECT id, password FROM users WHERE username = :username
        ");
        $stmt->execute([':username' => $username]);
        $user = $stmt->fetch(PDO::FETCH_ASSOC);
        
        if ($user && password_verify($password, $user['password'])) {
            return $user['id'];
        }
        return false;
    }
}

// Handle API requests
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $action = $_POST['action'] ?? '';
    $saveManager = new GameSaveManager();
    
    switch($action) {
        case 'save':
            $userId = $_POST['user_id'] ?? 1;
            $saveName = $_POST['save_name'] ?? 'Untitled Save';
            $gameData = $_POST['game_data'] ?? '{}';
            $result = $saveManager->saveGame($userId, $saveName, $gameData);
            echo json_encode(['success' => true, 'save_id' => $result]);
            break;
            
        case 'load':
            $saveId = $_POST['save_id'] ?? 0;
            $gameData = $saveManager->loadGame($saveId);
            echo json_encode(['success' => true, 'game_data' => $gameData]);
            break;
            
        case 'get_saves':
            $userId = $_POST['user_id'] ?? 1;
            $saves = $saveManager->getUserSaves($userId);
            echo json_encode(['success' => true, 'saves' => $saves]);
            break;
            
        case 'login':
            $username = $_POST['username'] ?? '';
            $password = $_POST['password'] ?? '';
            $userId = $saveManager->authenticateUser($username, $password);
            echo json_encode(['success' => (bool)$userId, 'user_id' => $userId]);
            break;
            
        case 'register':
            $username = $_POST['username'] ?? '';
            $password = $_POST['password'] ?? '';
            $userId = $saveManager->createUser($username, $password);
            echo json_encode(['success' => true, 'user_id' => $userId]);
            break;
    }
}
?>