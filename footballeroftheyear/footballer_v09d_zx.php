<?php
$teamsData = json_decode(file_get_contents('teams.txt'), true);
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <!--
    Footballer of the Year
    Changes:
    v09d_260107 - Fixed league table showing 0 games for teams not playing vs player: Now simulates ALL matches in player's division
    v09a_260107 - ZX Spectrum Edition: Incident cards, Transfer cards, 3-type penalty minigame (0,1,2 defenders), Weekly report, Customizable avatar, Quick match option, Enhanced menu grid, 25 incident types, Form tracking, Performance rating
    v008f3_260107 - ChatGPT 5 : Fixed end of seasson bug, goal bug
    v008f2_260107 - Fixed promotion/relegation to properly swap teams between divisions (bottom 2 swap with top 2 of lower division)
    v008f1_260106 - Team-based promotion/relegation (first 2 up, last 2 down), transfer offers with accept/decline, mid-season and end-of-season transfers, performance-based offers
    v008e_260106 - Round-robin league fixture system - fixed schedule, consistent results
    v008c_250104 - Next week on match exit, league standings with country/division dropdowns
    v008b_250104 - Country/Division/Team selection from teams.txt, removed position selection
    v008a_250104 - Improved goalkeeper visual feedback in penalty shootout
    v007_251225 - Goal Card system , League table fixed

    -->
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Footballer of the Year</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #fff;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .game-container {
            max-width: 800px;
            width: 100%;
            background: rgba(0, 0, 0, 0.8);
            border: 3px solid #00ff00;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 0 20px rgba(0, 255, 0, 0.3);
        }

        h1 {
            text-align: center;
            color: #00ff00;
            margin-bottom: 20px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
        }

        .screen {
            display: none;
        }

        .screen.active {
            display: block;
        }

        .btn {
            background: #00ff00;
            color: #000;
            border: none;
            padding: 10px 20px;
            font-family: 'Courier New', monospace;
            font-size: 16px;
            cursor: pointer;
            margin: 5px;
            border-radius: 5px;
            transition: all 0.3s;
        }

        .btn:hover {
            background: #00cc00;
            transform: scale(1.05);
        }

        .btn:disabled {
            background: #666;
            cursor: not-allowed;
            transform: none;
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }

        .stat-box {
            background: rgba(0, 255, 0, 0.1);
            padding: 15px;
            border: 2px solid #00ff00;
            border-radius: 5px;
            text-align: center;
        }

        .stat-label {
            color: #00ff00;
            font-size: 12px;
            margin-bottom: 5px;
        }

        .stat-value {
            font-size: 24px;
            font-weight: bold;
        }

        .player-name {
            text-align: center;
            font-size: 28px;
            margin-bottom: 20px;
            color: #ffff00;
        }

        input, select {
            background: #000;
            color: #fff;
            border: 2px solid #00ff00;
            padding: 10px;
            font-family: 'Courier New', monospace;
            font-size: 16px;
            width: 100%;
            margin: 10px 0;
            border-radius: 5px;
        }

        input:focus, select:focus {
            outline: none;
            border-color: #ffff00;
        }

        select {
            cursor: pointer;
        }

        .match-result {
            text-align: center;
            margin: 20px 0;
            padding: 20px;
            background: rgba(255, 255, 0, 0.1);
            border: 2px solid #ffff00;
            border-radius: 5px;
        }

        .score {
            font-size: 48px;
            color: #ffff00;
            margin: 10px 0;
        }

        .training-options {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 10px;
            margin: 20px 0;
        }

        .training-btn {
            background: rgba(0, 255, 0, 0.2);
            color: #fff;
            border: 2px solid #00ff00;
            padding: 15px;
            cursor: pointer;
            border-radius: 5px;
            transition: all 0.3s;
        }

        .training-btn:hover {
            background: rgba(0, 255, 0, 0.4);
        }

        .season-info {
            background: rgba(255, 255, 0, 0.1);
            padding: 15px;
            border: 2px solid #ffff00;
            border-radius: 5px;
            margin: 20px 0;
        }

        .transfer-list {
            max-height: 300px;
            overflow-y: auto;
            background: rgba(0, 0, 0, 0.5);
            padding: 10px;
        }

        .transfer-item {
            padding: 10px;
            margin: 5px 0;
            background: rgba(0, 255, 0, 0.1);
            border: 1px solid #00ff00;
            border-radius: 5px;
            cursor: pointer;
        }

        .transfer-item:hover {
            background: rgba(0, 255, 0, 0.2);
        }

        .menu-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 10px;
            margin: 20px 0;
        }

        .menu-btn {
            background: rgba(0, 255, 0, 0.2);
            color: #fff;
            border: 2px solid #00ff00;
            padding: 20px;
            font-size: 14px;
            cursor: pointer;
            border-radius: 5px;
            transition: all 0.3s;
        }

        .menu-btn:hover {
            background: rgba(0, 255, 0, 0.4);
        }

        .career-stats {
            margin: 20px 0;
        }

        .career-stat {
            display: flex;
            justify-content: space-between;
            padding: 5px 0;
            border-bottom: 1px solid #333;
        }

        ::-webkit-scrollbar {
            width: 10px;
        }

        ::-webkit-scrollbar-track {
            background: #000;
        }

        ::-webkit-scrollbar-thumb {
            background: #00ff00;
            border-radius: 5px;
        }

        .goal-cards {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin: 20px 0;
            flex-wrap: wrap;
        }

        .goal-card {
            background: linear-gradient(135deg, #ffd700 0%, #ffaa00 100%);
            color: #000;
            padding: 15px 25px;
            border: 3px solid #fff;
            border-radius: 10px;
            font-weight: bold;
            font-size: 18px;
            box-shadow: 0 0 10px rgba(255, 215, 0, 0.5);
        }

        .shop-item {
            background: rgba(0, 255, 0, 0.1);
            border: 2px solid #00ff00;
            border-radius: 5px;
            padding: 15px;
            margin: 10px 0;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .shop-item-info {
            flex: 1;
        }

        .shop-item-price {
            color: #ffd700;
            font-size: 20px;
            font-weight: bold;
        }

        .money-display {
            text-align: center;
            font-size: 24px;
            color: #ffd700;
            margin: 20px 0;
            padding: 10px;
            background: rgba(255, 215, 0, 0.1);
            border: 2px solid #ffd700;
            border-radius: 5px;
        }

        .penalty-game {
            background: linear-gradient(180deg, 
                #87CEEB 0%, 
                #87CEEB 30%,
                #228B22 30%,
                #1e7a1e 50%,
                #228B22 70%,
                #1e7a1e 100%
            );
            border: 3px solid #00ff00;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            position: relative;
            overflow: hidden;
        }

        .penalty-game::before {
            content: '';
            position: absolute;
            top: 30%;
            left: 0;
            right: 0;
            height: 70%;
            background: repeating-linear-gradient(
                90deg,
                transparent,
                transparent 50px,
                rgba(255,255,255,0.03) 50px,
                rgba(255,255,255,0.03) 51px
            );
            pointer-events: none;
        }

        .goal {
            position: relative;
            width: 300px;
            height: 180px;
            margin: 20px auto;
        }

        .goal-frame {
            position: absolute;
            width: 100%;
            height: 100%;
            background: linear-gradient(135deg, rgba(255,255,255,0.9) 0%, rgba(200,200,200,0.8) 100%);
            border: 8px solid #fff;
            border-bottom: none;
            border-radius: 5px 5px 0 0;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }

        .goal-line {
            position: absolute;
            bottom: 0;
            left: 0;
            width: 100%;
            height: 8px;
            background: linear-gradient(90deg, #fff 0%, #f0f0f0 50%, #fff 100%);
        }

        .goal-net {
            position: absolute;
            width: 100%;
            height: 100%;
            background-image: 
                repeating-linear-gradient(90deg, transparent, transparent 14px, rgba(0,0,0,0.1) 14px, rgba(0,0,0,0.1) 15px),
                repeating-linear-gradient(0deg, transparent, transparent 14px, rgba(0,0,0,0.1) 14px, rgba(0,0,0,0.1) 15px);
            pointer-events: none;
        }

        .goalkeeper {
            position: absolute;
            bottom: 10px;
            left: 50%;
            transform: translateX(-50%);
            width: 50px;
            height: 70px;
            transition: left 0.3s ease;
        }

        .gk-body {
            position: absolute;
            bottom: 0;
            width: 100%;
            height: 55px;
            background: linear-gradient(180deg, #FFD700 0%, #FFA500 100%);
            border-radius: 5px 5px 0 0;
            border: 2px solid #000;
        }

        .gk-jersey-num {
            position: absolute;
            top: 10px;
            left: 50%;
            transform: translateX(-50%);
            font-size: 16px;
            font-weight: bold;
            color: #000;
        }

        .gk-head {
            position: absolute;
            top: 0;
            left: 50%;
            transform: translateX(-50%);
            width: 20px;
            height: 20px;
            background: #FFDAB9;
            border-radius: 50%;
            border: 2px solid #000;
        }

        .gk-glove-left, .gk-glove-right {
            position: absolute;
            top: 15px;
            width: 18px;
            height: 22px;
            background: #FFD700;
            border: 2px solid #000;
            border-radius: 5px;
        }

        .gk-glove-left {
            left: -8px;
        }

        .gk-glove-right {
            right: -8px;
        }

        .defender {
            position: absolute;
            bottom: 20px;
            width: 40px;
            height: 60px;
            transition: all 0.4s ease;
            z-index: 5;
        }

        .def-head {
            position: absolute;
            top: 0;
            left: 50%;
            transform: translateX(-50%);
            width: 16px;
            height: 16px;
            background: #FFDAB9;
            border-radius: 50%;
            border: 2px solid #000;
        }

        .def-body {
            position: absolute;
            bottom: 0;
            width: 100%;
            height: 45px;
            background: linear-gradient(180deg, #4169E1 0%, #00008B 100%);
            border-radius: 5px 5px 0 0;
            border: 2px solid #000;
        }

        .def-jersey-num {
            position: absolute;
            top: 8px;
            left: 50%;
            transform: translateX(-50%);
            font-size: 14px;
            font-weight: bold;
            color: #fff;
        }

        .defender.blocking {
            animation: deflect 0.3s ease;
        }

        @keyframes deflect {
            0%, 100% { transform: translateX(-50%); }
            50% { transform: translateX(-50%) translateY(-20px); }
        }

        .ball {
            position: absolute;
            bottom: -80px;
            left: 50%;
            transform: translateX(-50%);
            width: 28px;
            height: 28px;
            background: radial-gradient(circle at 30% 30%, #fff, #ddd);
            border-radius: 50%;
            border: 2px solid #000;
            transition: all 0.6s cubic-bezier(0.25, 0.46, 0.45, 0.94);
            box-shadow: 2px 2px 5px rgba(0,0,0,0.3);
        }

        .ball::before {
            content: '';
            position: absolute;
            top: 3px;
            left: 3px;
            width: 8px;
            height: 8px;
            background: #000;
            border-radius: 50%;
            opacity: 0.5;
        }

        .shot-zones {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 8px;
            width: 320px;
            margin: 20px auto;
        }

        .shot-zone {
            background: rgba(0, 0, 0, 0.6);
            border: 3px solid #00ff00;
            padding: 12px 8px;
            cursor: pointer;
            text-align: center;
            font-weight: bold;
            font-size: 11px;
            transition: all 0.2s;
            border-radius: 5px;
            color: #fff;
            text-transform: uppercase;
        }

        .shot-zone:hover {
            background: rgba(0, 255, 0, 0.3);
            border-color: #ffff00;
            transform: scale(1.05);
            box-shadow: 0 0 10px rgba(0, 255, 0, 0.5);
        }

        .shot-zone.shot {
            background: rgba(100, 100, 100, 0.6);
            border-color: #666;
            cursor: not-allowed;
            color: #999;
            transform: none;
            box-shadow: none;
        }

        .penalty-result {
            text-align: center;
            font-size: 36px;
            font-weight: bold;
            margin: 20px 0;
            padding: 20px;
            border-radius: 10px;
            text-transform: uppercase;
            letter-spacing: 3px;
            animation: pulse 0.5s ease-in-out;
        }

        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }

        .penalty-result.goal {
            background: linear-gradient(135deg, rgba(0, 255, 0, 0.3) 0%, rgba(0, 200, 0, 0.2) 100%);
            color: #00ff00;
            border: 3px solid #00ff00;
            box-shadow: 0 0 20px rgba(0, 255, 0, 0.5);
        }

        .penalty-result.miss {
            background: linear-gradient(135deg, rgba(255, 0, 0, 0.3) 0%, rgba(200, 0, 0, 0.2) 100%);
            color: #ff6666;
            border: 3px solid #ff6666;
            box-shadow: 0 0 20px rgba(255, 0, 0, 0.5);
        }

        .goal-celebration {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-size: 80px;
            animation: celebrate 1s ease-out forwards;
            pointer-events: none;
            z-index: 100;
        }

        @keyframes celebrate {
            0% { transform: translate(-50%, -50%) scale(0); opacity: 0; }
            50% { transform: translate(-50%, -50%) scale(1.5); opacity: 1; }
            100% { transform: translate(-50%, -50%) scale(1); opacity: 0; }
        }

        .events {
            max-height: 150px;
            overflow-y: auto;
            background: rgba(0, 0, 0, 0.5);
            padding: 10px;
            border: 1px solid #00ff00;
            margin: 10px 0;
        }

        .event {
            margin: 5px 0;
            padding: 5px;
            border-bottom: 1px solid #333;
        }

        .goal-event {
            color: #00ff00;
        }

        .transfer-offer-card {
            background: rgba(0, 255, 0, 0.15);
            border: 3px solid #00ff00;
            border-radius: 10px;
            padding: 20px;
            margin: 15px 0;
            box-shadow: 0 0 15px rgba(0, 255, 0, 0.3);
        }

        .transfer-offer-header {
            color: #ffff00;
            font-size: 20px;
            font-weight: bold;
            margin-bottom: 15px;
            text-align: center;
        }

        .transfer-offer-details {
            margin: 10px 0;
            font-size: 16px;
        }

        .transfer-offer-details strong {
            color: #00ff00;
        }

        .transfer-actions {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin-top: 15px;
        }

        .accept-btn {
            background: #00ff00;
            color: #000;
        }

        .decline-btn {
            background: #ff0000;
            color: #fff;
        }

        .no-offers {
            text-align: center;
            color: #aaa;
            padding: 20px;
        }

        .player-avatar-container {
            text-align: center;
            margin-bottom: 20px;
        }

        .player-avatar {
            font-size: 60px;
            margin-bottom: 10px;
            display: inline-block;
            width: 70px;
            height: 70px;
            line-height: 70px;
            background: rgba(0, 255, 0, 0.1);
            border: 3px solid #00ff00;
            border-radius: 50%;
        }

        .report-section {
            background: rgba(0, 0, 0, 0.5);
            border: 2px solid #00ff00;
            border-radius: 5px;
            padding: 15px;
            margin: 15px 0;
        }

        .report-section h3 {
            color: #00ff00;
            margin-bottom: 10px;
        }

        .form-display {
            display: flex;
            gap: 10px;
            justify-content: center;
        }

        .form-badge {
            width: 40px;
            height: 40px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-weight: bold;
            border-radius: 5px;
            border: 2px solid #000;
        }

        .form-badge.win {
            background: #00ff00;
            color: #000;
        }

        .form-badge.draw {
            background: #ffff00;
            color: #000;
        }

        .form-badge.loss {
            background: #ff6666;
            color: #000;
        }

        .card-section {
            background: rgba(0, 0, 0, 0.5);
            border: 2px solid #00ff00;
            border-radius: 5px;
            padding: 15px;
            margin: 15px 0;
        }

        .card-section h3 {
            color: #00ff00;
            margin-bottom: 10px;
        }

        .card-info {
            background: rgba(255, 215, 0, 0.1);
            border: 2px solid #ffd700;
            border-radius: 5px;
            padding: 15px;
            margin: 20px 0;
            font-size: 14px;
        }

        .emoji-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 10px;
            margin: 15px 0;
        }

        .emoji-grid button {
            font-size: 30px;
            padding: 10px;
            background: rgba(0, 255, 0, 0.1);
            border: 2px solid #00ff00;
            border-radius: 5px;
            cursor: pointer;
        }

        .emoji-grid button:hover {
            background: rgba(0, 255, 0, 0.3);
        }
    </style>
</head>
<body>
    <div class="game-container">
        <h1>⚽ Footballer of the Year ⚽</h1>

        <div id="startScreen" class="screen active">
            <div style="text-align: center; margin: 30px 0;">
                <p style="font-size: 18px; margin-bottom: 20px;">
                    Create your footballer and lead them to glory!
                </p>
                <input type="text" id="playerName" placeholder="Enter your name" maxlength="20">
                
                <div style="margin-top: 20px; text-align: left;">
                    <label style="font-size: 14px; color: #aaa;">Select Country:</label>
                    <select id="countrySelect" onchange="onCountryChange()">
                        <option value="">-- Select Country --</option>
                        <?php foreach($teamsData['countries'] as $country): ?>
                            <option value="<?php echo htmlspecialchars($country['name']); ?>">
                                <?php echo htmlspecialchars($country['name']); ?>
                            </option>
                        <?php endforeach; ?>
                    </select>
                </div>
                
                <div style="margin-top: 10px; text-align: left;">
                    <label style="font-size: 14px; color: #aaa;">Select Division:</label>
                    <select id="divisionSelect" onchange="onDivisionChange()" disabled>
                        <option value="">-- Select Division --</option>
                    </select>
                </div>
                
                <div style="margin-top: 10px; text-align: left;">
                    <label style="font-size: 14px; color: #aaa;">Select Team:</label>
                    <select id="teamSelect" disabled>
                        <option value="">-- Select Team --</option>
                    </select>
                </div>
                
                <div style="margin-top: 30px; text-align: center;">
                    <button class="btn" id="startBtn" onclick="createPlayer()" disabled>
                        Start Career
                    </button>
                </div>
            </div>
        </div>

        <div id="mainMenu" class="screen">
            <div class="player-avatar-container">
                <div class="player-avatar" id="menuAvatar">⚽</div>
                <div class="player-name" id="menuPlayerName"></div>
            </div>
            <div class="season-info">
                <p>Season: <span id="menuSeason">1</span> | Week: <span id="menuWeek">1</span></p>
                <p>Country: <span id="menuCountry"></span></p>
                <p>Division: <span id="menuDivision"></span></p>
                <p>Club: <span id="menuClub"></span></p>
                <p>Overall: <span id="menuOverall"></span></p>
            </div>
            <div class="money-display">
                💰 £<span id="menuMoney">500</span> (Weekly: £<span id="menuSalary">50</span>)
            </div>
            <div class="goal-cards">
                <div class="goal-card">🎯 Goal: <span id="menuGoalCards">3</span> | ❓ Incident: <span id="menuIncidentCards">3</span> | 🔄 Transfer: <span id="menuTransferCards">1</span></div>
                <div class="goal-card" style="background: linear-gradient(135deg, #87CEEB 0%, #4169E1 100%);">🗳️ Votes: <span id="menuVotes">0</span></div>
            </div>
            <div id="incidentDisplay" class="match-result" style="display: none;"></div>
            <div class="menu-grid">
                <button class="menu-btn" onclick="showScreen('statsScreen')">
                    📊 Stats
                </button>
                <button class="menu-btn" onclick="showScreen('trainingScreen')">
                    🏋️ Training
                </button>
                <button class="menu-btn" onclick="showScreen('leagueScreen')">
                    📊 League
                </button>
                <button class="menu-btn" onclick="showScreen('matchSelectionScreen')">
                    ⚽ Select
                </button>
                <button class="menu-btn" onclick="showScreen('matchScreen')">
                    ⚽ Play
                </button>
                <button class="menu-btn" onclick="showScreen('shopScreen')">
                    🛒 Shop
                </button>
                <button class="menu-btn" onclick="checkTransferOffers()">
                    🔄 Transfers
                </button>
                <button class="menu-btn" onclick="showScreen('cardManagementScreen')">
                    🃏 Cards
                </button>
                <button class="menu-btn" onclick="showScreen('careerScreen')">
                    🏆 Career
                </button>
                <button class="menu-btn" onclick="showScreen('cupScreen')">
                    🏆 Cups
                </button>
                <button class="menu-btn" onclick="nextWeek()">
                    📅 Next Week
                </button>
                <button class="menu-btn" onclick="showScreen('settingsScreen')">
                    ⚙️ Settings
                </button>
            </div>
        </div>

        <div id="statsScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Player Stats</h2>
            <div class="player-name" id="statsPlayerName"></div>
            <div class="stats-grid">
                <div class="stat-box">
                    <div class="stat-label">Overall</div>
                    <div class="stat-value" id="statOverall">60</div>
                </div>
                <div class="stat-box">
                    <div class="stat-label">Speed</div>
                    <div class="stat-value" id="statSpeed">60</div>
                </div>
                <div class="stat-box">
                    <div class="stat-label">Shooting</div>
                    <div class="stat-value" id="statShooting">60</div>
                </div>
                <div class="stat-box">
                    <div class="stat-label">Passing</div>
                    <div class="stat-value" id="statPassing">60</div>
                </div>
                <div class="stat-box">
                    <div class="stat-label">Defense</div>
                    <div class="stat-value" id="statDefense">60</div>
                </div>
                <div class="stat-box">
                    <div class="stat-label">Stamina</div>
                    <div class="stat-value" id="statStamina">60</div>
                </div>
            </div>
            <div style="text-align: center; margin-top: 20px;">
                <p>Age: <span id="statAge"></span></p>
                <p>Energy: <span id="statEnergy"></span>%</p>
            </div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="trainingScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Training</h2>
            <p style="text-align: center; margin-bottom: 20px;">Energy: <span id="trainingEnergy"></span>%</p>
            <div class="training-options">
                <button class="training-btn" onclick="train('speed')">
                    <strong>Speed Training</strong><br>+2 Speed<br>-10 Energy
                </button>
                <button class="training-btn" onclick="train('shooting')">
                    <strong>Shooting Practice</strong><br>+2 Shooting<br>-10 Energy
                </button>
                <button class="training-btn" onclick="train('passing')">
                    <strong>Passing Drills</strong><br>+2 Passing<br>-10 Energy
                </button>
                <button class="training-btn" onclick="train('defense')">
                    <strong>Defense Training</strong><br>+2 Defense<br>-10 Energy
                </button>
                <button class="training-btn" onclick="train('stamina')">
                    <strong>Stamina Building</strong><br>+2 Stamina<br>-10 Energy
                </button>
                <button class="training-btn" onclick="train('rest')">
                    <strong>Rest & Recover</strong><br>+30 Energy<br>No stat boost
                </button>
            </div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="matchScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Match Day</h2>
            <div class="season-info">
                <p>Week <span id="matchWeek"></span></p>
                <p><span id="homeTeam"></span> vs <span id="awayTeam"></span></p>
            </div>
            <div class="goal-cards">
                <div class="goal-card">🎯 Goal Cards: <span id="matchGoalCards">3</span></div>
            </div>
            <div class="match-result" id="matchResult" style="display: none;">
                <div class="score" id="finalScore">0 - 0</div>
                <div id="matchOutcome"></div>
            </div>
            <div class="events" id="matchEvents"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" id="useCardBtn" onclick="useGoalCard()">Use Goal Card</button>
                <button class="btn" id="endMatchBtn" onclick="endMatch()" style="display: none;">End Match</button>
                <button class="btn" onclick="exitMatchAndNextWeek()">Back</button>
            </div>
        </div>

        <div id="shopScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Shop</h2>
            <p style="text-align: center; margin-bottom: 10px;">Division: <span id="shopDivision"></span></p>
            <p style="text-align: center; margin-bottom: 10px; color: #ffd700;">Goal Card Price: £<span id="shopCardPrice">500</span></p>
            <div class="money-display">
                💰 £<span id="shopMoney">500</span>
            </div>
            <div id="shopItems"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="transferScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Transfer Market</h2>
            <p style="text-align: center; margin-bottom: 20px;">Current Club: <span id="currentClub"></span></p>
            <p style="text-align: center; margin-bottom: 10px;">Current Value: $<span id="currentValue">0</span>M</p>
            <div class="transfer-list" id="transferList"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="transferOfferScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Transfer Offers</h2>
            <div id="transferOfferList"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="closeTransferOffers()">Back</button>
            </div>
        </div>

        <div id="careerScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Career Stats</h2>
            <div class="player-name" id="careerPlayerName"></div>
            <div class="career-stats">
                <div class="career-stat">
                    <span>Seasons Played:</span>
                    <span id="careerSeasons">0</span>
                </div>
                <div class="career-stat">
                    <span>Total Goals:</span>
                    <span id="careerGoals">0</span>
                </div>
                <div class="career-stat">
                    <span>Matches Played:</span>
                    <span id="careerMatches">0</span>
                </div>
                <div class="career-stat">
                    <span>Trophies Won:</span>
                    <span id="careerTrophies">0</span>
                </div>
                <div class="career-stat">
                    <span>Clubs Played For:</span>
                    <span id="careerClubs">0</span>
                </div>
                <div class="career-stat">
                    <span>Total Votes:</span>
                    <span id="careerVotes">0</span>
                </div>
                <div class="career-stat">
                    <span>Highest Division:</span>
                    <span id="careerHighestDiv"></span>
                </div>
            </div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="leagueScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">League Standings</h2>
            <div style="display: flex; gap: 10px; justify-content: center; align-items: center; margin-bottom: 20px; flex-wrap: wrap;">
                <select id="leagueCountrySelect" onchange="onLeagueCountryChange()" style="max-width: 200px; background: #000; color: #fff; border: 2px solid #00ff00; padding: 8px; font-family: 'Courier New', monospace; border-radius: 5px;">
                    <option value="">-- Select Country --</option>
                </select>
                <select id="leagueDivisionSelect" onchange="onLeagueDivisionChange()" style="max-width: 200px; background: #000; color: #fff; border: 2px solid #00ff00; padding: 8px; font-family: 'Courier New', monospace; border-radius: 5px;">
                    <option value="">-- Select Division --</option>
                </select>
                <button class="btn" onclick="returnToMyDivision()">Return to My Division</button>
            </div>
            <p style="text-align: center; margin-bottom: 10px;">Division: <span id="leagueDivision"></span></p>
            <div style="overflow-x: auto; margin-bottom: 20px;">
                <table style="width: 100%; border-collapse: collapse; background: rgba(0, 0, 0, 0.5); border: 2px solid #00ff00;">
                    <thead>
                        <tr style="background: rgba(0, 255, 0, 0.2);">
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">Pos</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">Team</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">P</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">W</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">D</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">L</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">GF</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">GA</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">GD</th>
                            <th style="padding: 10px; border: 1px solid #00ff00; color: #fff;">Pts</th>
                        </tr>
                    </thead>
                    <tbody id="leagueTableBody"></tbody>
                </table>
            </div>
            <div id="weekResults" style="max-height: 200px; overflow-y: auto; background: rgba(0, 0, 0, 0.5); padding: 10px; border: 1px solid #00ff00; margin-bottom: 20px;"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="settingsScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Settings</h2>
            
            <div class="season-info">
                <h3 style="color: #00ff00; margin-bottom: 10px;">💾 Save & Load Game</h3>
                <p id="saveStatus" style="text-align: center; margin-bottom: 10px; color: #aaa;">Checking for saved game...</p>
                <div style="display: flex; gap: 10px; justify-content: center; margin-bottom: 20px; flex-wrap: wrap;">
                    <button class="btn" onclick="saveGame()">Save Game</button>
                    <button class="btn" onclick="loadGame()">Load Game</button>
                    <button class="btn" onclick="exportGame()">Export to File</button>
                    <button class="btn" onclick="document.getElementById('importFile').click()">Import from File</button>
                    <input type="file" id="importFile" accept=".json" style="display: none;" onchange="importGame(event)">
                </div>
            </div>
            
            <div class="season-info">
                <h3 style="color: #00ff00; margin-bottom: 10px;">👤 Player Avatar</h3>
                <div class="player-avatar-container">
                    <div class="player-avatar" id="settingsAvatar">⚽</div>
                </div>
                <input type="text" id="avatarUrlInput" placeholder="Image URL (optional)" style="margin: 10px 0;">
                <div style="display: flex; gap: 10px; justify-content: center; margin: 10px 0;">
                    <button class="btn" onclick="updateAvatar()">Update Avatar</button>
                </div>
                <p style="text-align: center; font-size: 14px; color: #aaa;">Or choose an emoji:</p>
                <div class="emoji-grid">
                    <button onclick="setAvatar('⚽')">⚽</button>
                    <button onclick="setAvatar('🏃')">🏃</button>
                    <button onclick="setAvatar('🥅')">🥅</button>
                    <button onclick="setAvatar('🧢')">🧢</button>
                    <button onclick="setAvatar('🎯')">🎯</button>
                    <button onclick="setAvatar('⭐')">⭐</button>
                    <button onclick="setAvatar('🌟')">🌟</button>
                    <button onclick="setAvatar('🔥')">🔥</button>
                    <button onclick="setAvatar('😎')">😎</button>
                    <button onclick="setAvatar('😤')">😤</button>
                    <button onclick="setAvatar('💪')">💪</button>
                </div>
            </div>

            <div class="season-info">
                <h3 style="color: #00ff00; margin-bottom: 10px;">⚡ Quick Match Mode</h3>
                <p style="text-align: center; margin-bottom: 10px;">Skip weekly report after Next Week</p>
                <div style="display: flex; gap: 10px; justify-content: center;">
                    <button class="btn" id="quickMatchBtn" onclick="toggleQuickMatch()">Currently: OFF</button>
                </div>
            </div>

            <div class="season-info">
                <h3 style="color: #00ff00; margin-bottom: 10px;">⚠️ Danger Zone</h3>
                <div style="display: flex; gap: 10px; justify-content: center;">
                    <button class="btn" onclick="resetGame()" style="background: #ff0000;">Reset Game</button>
                </div>
            </div>
            
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="matchSelectionScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Select Matches This Season</h2>
            <p style="text-align: center; margin-bottom: 10px;">Division: <span id="selectDivision"></span></p>
            <p style="text-align: center; margin-bottom: 20px;">Select up to 10 matches. Fewer matches = each goal card worth more votes!</p>
            <div id="matchList" style="max-height: 400px; overflow-y: auto; margin-bottom: 20px;"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="confirmMatchSelection()">Confirm Selection</button>
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="cupScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Cup Competitions</h2>
            <div class="stats-grid">
                <div class="stat-box">
                    <div class="stat-label">FA Cup</div>
                    <div class="stat-value" id="faCupStatus">Not Qualified</div>
                </div>
                <div class="stat-box">
                    <div class="stat-label">European Cup</div>
                    <div class="stat-value" id="euroCupStatus">Not Qualified</div>
                </div>
            </div>
            <p style="text-align: center; margin-top: 20px; color: #aaa;">Cup competitions qualify at Division 2 and above</p>
            <div id="cupActions" style="text-align: center; margin-top: 20px;"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="penaltyScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 10px;">Penalty Shootout!</h2>
            <p style="text-align: center; margin-bottom: 5px; font-size: 18px;">Type: <span id="penaltyType" style="color: #ffd700; font-weight: bold;">Type 1 - No Defenders</span></p>
            <p style="text-align: center; margin-bottom: 5px; font-size: 18px;">Shooting: <span id="penaltyShooting" style="color: #ffd700; font-weight: bold;">60</span></p>
            <p style="text-align: center; margin-bottom: 15px; font-size: 14px; color: #aaa;">Choose a target zone to shoot!</p>
            <div class="penalty-game" id="penaltyGame">
                <div class="goal">
                    <div class="goal-frame">
                        <div class="goal-net"></div>
                        <div class="goal-line"></div>
                    </div>
                    <div class="goalkeeper" id="goalkeeper">
                        <div class="gk-head"></div>
                        <div class="gk-body">
                            <div class="gk-jersey-num">1</div>
                        </div>
                        <div class="gk-glove-left"></div>
                        <div class="gk-glove-right"></div>
                    </div>
                    <div class="defender" id="defender1" style="display: none;">
                        <div class="def-head"></div>
                        <div class="def-body">
                            <div class="def-jersey-num">4</div>
                        </div>
                    </div>
                    <div class="defender" id="defender2" style="display: none;">
                        <div class="def-head"></div>
                        <div class="def-body">
                            <div class="def-jersey-num">5</div>
                        </div>
                    </div>
                    <div class="ball" id="ball"></div>
                </div>
                <div class="shot-zones" id="shotZones">
                    <div class="shot-zone" data-zone="0">↖️ TOP LEFT</div>
                    <div class="shot-zone" data-zone="1">⬆️ TOP CENTER</div>
                    <div class="shot-zone" data-zone="2">↗️ TOP RIGHT</div>
                    <div class="shot-zone" data-zone="3">⬅️ MID LEFT</div>
                    <div class="shot-zone" data-zone="4">⏺️ CENTER</div>
                    <div class="shot-zone" data-zone="5">➡️ MID RIGHT</div>
                    <div class="shot-zone" data-zone="6">↙️ BOTTOM LEFT</div>
                    <div class="shot-zone" data-zone="7">⬇️ BOTTOM CENTER</div>
                    <div class="shot-zone" data-zone="8">↘️ BOTTOM RIGHT</div>
                </div>
            </div>
            <div class="penalty-result" id="penaltyResult" style="display: none;"></div>
            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" id="backToMatchBtn" onclick="backToMatch()" style="display: none;">Back to Match</button>
            </div>
        </div>

        <div id="cardManagementScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Card Management</h2>

            <div class="card-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">🎯 Goal Cards</h3>
                <p style="text-align: center;">You have: <span id="mgGoalCards" style="color: #ffd700; font-weight: bold; font-size: 24px;">0</span></p>
                <p style="text-align: center; font-size: 14px; color: #aaa;">Use in matches to take penalty shots!</p>
            </div>

            <div class="card-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">❓ Incident Cards</h3>
                <p style="text-align: center;">You have: <span id="mgIncidentCards" style="color: #ffd700; font-weight: bold; font-size: 24px;">0</span></p>
                <div style="text-align: center; margin: 15px 0;">
                    <button class="btn" onclick="useIncidentCard()">Use Incident Card</button>
                </div>
                <p style="text-align: center; font-size: 14px; color: #aaa;">Trigger random events (good or bad!)</p>
            </div>

            <div class="card-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">🔄 Transfer Cards</h3>
                <p style="text-align: center;">You have: <span id="mgTransferCards" style="color: #ffd700; font-weight: bold; font-size: 24px;">0</span></p>
                <div style="text-align: center; margin: 15px 0;">
                    <button class="btn" onclick="useTransferCard()">Use Transfer Card</button>
                </div>
                <p style="text-align: center; font-size: 14px; color: #aaa;">Get offers from interested clubs!</p>
            </div>

            <div class="card-info">
                <p style="text-align: center; margin-bottom: 10px;"><strong>💡 Tip:</strong> More difficult penalty types give better rewards!</p>
                <div style="text-align: left; margin: 10px 0;">
                    <p>🟢 Type 1 (0 defenders): £100 + normal votes</p>
                    <p>🟡 Type 2 (1 defender): £150 + 1.5x votes</p>
                    <p>🔴 Type 3 (2 defenders): £250 + 2x votes</p>
                </div>
            </div>

            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Back</button>
            </div>
        </div>

        <div id="weeklyReportScreen" class="screen">
            <h2 style="text-align: center; color: #00ff00; margin-bottom: 20px;">Weekly Report - Week <span id="reportWeek">1</span></h2>

            <div class="report-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">⚽ Match Performance</h3>
                <p>Opponent: <span id="reportOpponent" style="color: #ffd700; font-weight: bold;">-</span></p>
                <p>Result: <span id="reportResult" style="color: #ffd700; font-weight: bold;">-</span></p>
                <p>Performance Rating: <span id="reportRating" style="color: #ffd700; font-weight: bold;">-</span> / 10</p>
                <p>Goals: <span id="reportGoals" style="color: #ffd700; font-weight: bold;">0</span> | Shots: <span id="reportShots" style="color: #ffd700; font-weight: bold;">0</span> | On Target: <span id="reportOnTarget" style="color: #ffd700; font-weight: bold;">0</span></p>
            </div>

            <div class="report-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">📊 Form Guide (Last 5)</h3>
                <div id="formDisplay" style="display: flex; gap: 10px; justify-content: center;">
                </div>
            </div>

            <div class="report-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">🏆 League Position</h3>
                <p>Current: <span id="reportPosition" style="color: #ffd700; font-weight: bold;">-</span> (<span id="reportPositionChange" style="color: #aaa;">-</span>)</p>
                <p>Points: <span id="reportPoints" style="color: #ffd700; font-weight: bold;">0</span> | GD: <span id="reportGD" style="color: #ffd700; font-weight: bold;">0</span></p>
            </div>

            <div class="report-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">💰 Value Update</h3>
                <p>Previous: £<span id="reportOldValue">-</span>M → Current: £<span id="reportNewValue" style="color: #ffd700; font-weight: bold;">-</span>M</p>
                <p id="reportValueChange" style="color: #00ff00; font-weight: bold;">+£0M</p>
            </div>

            <div class="report-section">
                <h3 style="color: #00ff00; margin-bottom: 10px;">📋 Incidents This Week</h3>
                <p id="reportIncident" style="color: #aaa;">No incidents</p>
            </div>

            <div style="text-align: center; margin-top: 20px;">
                <button class="btn" onclick="showScreen('mainMenu')">Continue</button>
            </div>
        </div>
    </div>

    <script>
        const teamsData = <?php echo json_encode($teamsData); ?>;
        
        let divisions = {};
        let player = {
            name: '',
            country: '',
            division: 4,
            divisionName: '',
            age: 17,
            energy: 100,
            stats: {
                speed: 55,
                shooting: 55,
                passing: 55,
                defense: 55,
                stamina: 55
            },
            career: {
                goals: 0,
                matches: 0,
                trophies: 0,
                clubs: [],
                votes: 0,
                totalGoalsAllTime: 0
            },
            currentClub: '',
            season: 1,
            week: 1,
            value: 1,
            money: 500,
            goalCards: 10,
            weeklySalary: 50,
            selectedMatches: [],
            incidentCard: null,
            injured: false,
            weeksInjured: 0,
            incidentCards: 3,
            transferCards: 1,
            avatar: '⚽',
            avatarUrl: '',
            form: [],
            reputation: 50,
            quickMatch: false
        };

        let leagueView = {
            country: null,
            division: null
        };

        let transferOffers = [];
        let pendingTransferOffer = null;
        let midSeasonOfferReceived = false;

        function onCountryChange() {
            const countrySelect = document.getElementById('countrySelect');
            const divisionSelect = document.getElementById('divisionSelect');
            const teamSelect = document.getElementById('teamSelect');
            const startBtn = document.getElementById('startBtn');
            
            const selectedCountry = countrySelect.value;
            
            divisionSelect.innerHTML = '<option value="">-- Select Division --</option>';
            divisionSelect.disabled = true;
            teamSelect.innerHTML = '<option value="">-- Select Team --</option>';
            teamSelect.disabled = true;
            startBtn.disabled = true;
            
            if (selectedCountry) {
                const countryData = teamsData.countries.find(c => c.name === selectedCountry);
                if (countryData) {
                    const divisionsList = [];
                    for (let i = 1; i <= 4; i++) {
                        if (countryData.divisions[i]) {
                            divisionsList.push({
                                num: i,
                                name: countryData.divisions[i].name
                            });
                        }
                    }
                    
                    divisionsList.forEach(div => {
                        const option = document.createElement('option');
                        option.value = div.num;
                        option.textContent = `${div.num} - ${div.name}`;
                        divisionSelect.appendChild(option);
                    });
                    
                    divisionSelect.disabled = false;
                }
            }
        }

        function onDivisionChange() {
            const countrySelect = document.getElementById('countrySelect');
            const divisionSelect = document.getElementById('divisionSelect');
            const teamSelect = document.getElementById('teamSelect');
            const startBtn = document.getElementById('startBtn');
            
            const selectedCountry = countrySelect.value;
            const selectedDivision = parseInt(divisionSelect.value);
            
            teamSelect.innerHTML = '<option value="">-- Select Team --</option>';
            teamSelect.disabled = true;
            startBtn.disabled = true;
            
            if (selectedCountry && selectedDivision) {
                const countryData = teamsData.countries.find(c => c.name === selectedCountry);
                if (countryData && countryData.divisions[selectedDivision]) {
                    const teams = countryData.divisions[selectedDivision].teams;
                    
                    teams.forEach(team => {
                        const option = document.createElement('option');
                        option.value = team.name;
                        option.textContent = `${team.name} (Rating: ${team.rating})`;
                        teamSelect.appendChild(option);
                    });
                    
                    teamSelect.disabled = false;
                }
            }
        }

        document.getElementById('teamSelect').addEventListener('change', function() {
            const countrySelect = document.getElementById('countrySelect');
            const divisionSelect = document.getElementById('divisionSelect');
            const teamSelect = document.getElementById('teamSelect');
            const startBtn = document.getElementById('startBtn');
            
            startBtn.disabled = !(countrySelect.value && divisionSelect.value && teamSelect.value);
        });

        let leagueTables = {};
        let leagueFixtures = {};
        let matchState = {
            teamScore: 0,
            opponentScore: 0,
            playerGoals: 0,
            playerGoalsThisMatch: 0,
            isHome: true,
            opponentName: '',
            isPlaying: false,
            penaltyType: 1,
            defenderPositions: [],
            shots: 0,
            shotsOnTarget: 0,
            performanceRating: 0,
            weeklyReportData: {}
        };
        let matchSchedule = [];
        let cupState = {
            faCup: { qualified: false, round: '', opponent: '' },
            euroCup: { qualified: false, round: '', opponent: '' }
        };

        const incidentCards = [
            { type: 'positive', text: 'TV Appearance bonus!', effect: () => { player.money += 1000; return '£1000 bonus!'; }, weight: 1 },
            { type: 'positive', text: 'Sponsor deal offer!', effect: () => { player.money += 500; return '£500 bonus!'; }, weight: 2 },
            { type: 'positive', text: 'Fan favorite!', effect: () => { player.career.votes += 5; return '+5 votes!'; }, weight: 2 },
            { type: 'positive', text: 'Scout from top division watching!', effect: () => { player.reputation = Math.min(100, player.reputation + 10); return 'Reputation +10! Better transfer offers coming!'; }, weight: 0.5 },
            { type: 'positive', text: 'Training breakthrough!', effect: () => { const stats = ['speed', 'shooting', 'passing', 'defense', 'stamina']; player.stats[stats[Math.floor(Math.random() * stats.length)]] = Math.min(99, player.stats[stats[Math.floor(Math.random() * stats.length)]] + 2); return 'Random stat +2!'; }, weight: 0.5 },
            { type: 'positive', text: 'International call-up!', effect: () => { player.money += 300; player.reputation = Math.min(100, player.reputation + 5); return '£300 bonus, reputation +5!'; }, weight: 0.3 },
            { type: 'positive', text: 'Personal sponsorship!', effect: () => { player.weeklySalary += 50; return '+£50/week permanent salary!'; }, weight: 0.5 },
            { type: 'positive', text: 'Dietician advice!', effect: () => { player.stats.stamina = Math.min(99, player.stats.stamina + 5); return 'Stamina +5!'; }, weight: 0.5 },
            { type: 'positive', text: 'Psychologist session!', effect: () => { player.reputation = Math.min(100, player.reputation + 5); return 'Reputation +5, less injury chance!'; }, weight: 0.3 },
            { type: 'negative', text: 'Booked for diving!', effect: () => { player.money -= 200; player.career.votes -= 2; return '£200 fine, -2 votes'; }, weight: 1 },
            { type: 'negative', text: 'Minor injury (1 week)', effect: () => { player.injured = true; player.weeksInjured = 1; return 'Injured for 1 week!'; }, weight: 0.3 },
            { type: 'negative', text: 'Training injury (2 weeks)', effect: () => { player.injured = true; player.weeksInjured = 2; return 'Injured for 2 weeks!'; }, weight: 0.1 },
            { type: 'negative', text: 'Media scandal!', effect: () => { player.money -= 300; player.reputation = Math.max(0, player.reputation - 10); player.career.votes -= 3; return '£300 fine, reputation -10, -3 votes'; }, weight: 0.3 },
            { type: 'negative', text: 'Fan protest!', effect: () => { player.reputation = Math.max(0, player.reputation - 5); player.career.votes -= 2; return 'Reputation -5, -2 votes'; }, weight: 0.3 },
            { type: 'negative', text: 'Club financial troubles!', effect: () => { player.weeklySalary = Math.max(20, Math.floor(player.weeklySalary * 0.8)); return 'Salary reduced by 20%!'; }, weight: 0.2 },
            { type: 'negative', text: 'Suspension warning!', effect: () => { player.reputation = Math.max(0, player.reputation - 3); return 'Yellow card! Be careful!'; }, weight: 0.4 },
            { type: 'negative', text: 'Red card!', effect: () => { player.money -= 500; player.career.votes -= 5; return 'Suspended! £500 fine, -5 votes'; }, weight: 0.1 },
            { type: 'negative', text: 'Training setback!', effect: () => { const stats = ['speed', 'shooting', 'passing', 'defense', 'stamina']; player.stats[stats[Math.floor(Math.random() * stats.length)]] = Math.max(10, player.stats[stats[Math.floor(Math.random() * stats.length)]] - 1); return 'Random stat -1!'; }, weight: 0.4 },
            { type: 'negative', text: 'Tax investigation!', effect: () => { player.money -= 500; return '£500 tax fine!'; }, weight: 0.2 },
            { type: 'negative', text: 'Retirement rumors!', effect: () => { player.reputation = Math.max(0, player.reputation - 3); return 'Media speculation, reputation -3'; }, weight: 0.3 },
            { type: 'neutral', text: 'Contract negotiation talks', effect: () => { return 'Transfer offers may be coming...'; }, weight: 1 },
            { type: 'neutral', text: 'Fan club appreciation day!', effect: () => { player.career.votes += 2; return '+2 votes!'; }, weight: 1.5 },
            { type: 'neutral', text: 'Doping test negative!', effect: () => { player.reputation = Math.min(100, player.reputation + 3); return 'Reputation +3!'; }, weight: 0.8 },
            { type: 'neutral', text: 'Quiet week', effect: () => { return 'No incidents this week'; }, weight: 4 }
        ];

        function createPlayer() {
            const nameInput = document.getElementById('playerName');
            const countrySelect = document.getElementById('countrySelect');
            const divisionSelect = document.getElementById('divisionSelect');
            const teamSelect = document.getElementById('teamSelect');
            
            if (!nameInput.value.trim()) {
                alert('Please enter your name!');
                return;
            }
            if (!countrySelect.value) {
                alert('Please select a country!');
                return;
            }
            if (!divisionSelect.value) {
                alert('Please select a division!');
                return;
            }
            if (!teamSelect.value) {
                alert('Please select a team!');
                return;
            }

            player.name = nameInput.value.trim();
            player.country = countrySelect.value;
            player.division = parseInt(divisionSelect.value);
            player.divisionName = divisionSelect.options[divisionSelect.selectedIndex].text;
            player.currentClub = teamSelect.value;
            player.career.clubs.push(player.currentClub);
            
            const avatars = ['⚽', '🏃', '🥅', '🧢', '🎯', '⭐', '🌟', '🔥'];
            player.avatar = avatars[Math.floor(Math.random() * avatars.length)];
            
            initializeDivisions();
            initializeLeagueTables();
            generateMatchSchedule();
            updateAllDisplays();
            showScreen('mainMenu');
        }

        function initializeDivisions() {
            teamsData.countries.forEach(country => {
                Object.keys(country.divisions).forEach(divNum => {
                    const divKey = `${country.name}-${divNum}`;
                    const divData = country.divisions[divNum];
                    divisions[divKey] = {
                        name: divData.name,
                        goalCardPrice: divData.goalCardPrice,
                        country: country.name,
                        division: parseInt(divNum),
                        teams: divData.teams.map(t => t.name)
                    };
                });
            });
        }

        function getCurrentDivisionKey() {
            return `${player.country}-${player.division}`;
        }

        function generateRoundRobinFixtures(teamNames) {
            const teams = [...teamNames];
            const numTeams = teams.length;
            
            if (numTeams < 2) {
                return [];
            }
            
            const fixtures = [];
            const rounds = numTeams - 1;
            const matchesPerRound = Math.floor(numTeams / 2);
            
            for (let round = 0; round < rounds; round++) {
                const roundFixtures = [];
                
                for (let i = 0; i < matchesPerRound; i++) {
                    const home = teams[i];
                    const away = teams[numTeams - 1 - i];
                    roundFixtures.push({ home, away });
                }
                
                fixtures.push(roundFixtures);
                
                teams.splice(1, 0, teams.pop());
            }
            
            return fixtures;
        }

        function generateDoubleRoundRobinFixtures(teamNames) {
            const firstHalf = generateRoundRobinFixtures(teamNames);
            const secondHalf = firstHalf.map(round => 
                round.map(match => ({ home: match.away, away: match.home }))
            );
            
            return [...firstHalf, ...secondHalf];
        }

        function initializeLeagueFixtures() {
            teamsData.countries.forEach(country => {
                Object.keys(country.divisions).forEach(divNum => {
                    const divKey = `${country.name}-${divNum}`;
                    const divData = country.divisions[divNum];
                    const teams = divData.teams.map(t => t.name);
                    
                    leagueFixtures[divKey] = generateDoubleRoundRobinFixtures(teams);
                });
            });
        }

        function generateMatchSchedule() {
            matchSchedule = [];
            const divKey = getCurrentDivisionKey();
            const fixtures = leagueFixtures[divKey];
            
            if (!fixtures) {
                console.error('No fixtures found for division:', divKey);
                return;
            }
            
            fixtures.forEach((round, index) => {
                const playerMatch = round.find(m => 
                    m.home === player.currentClub || m.away === player.currentClub
                );
                
                if (playerMatch) {
                    matchSchedule.push({
                        week: index + 1,
                        opponent: playerMatch.home === player.currentClub ? playerMatch.away : playerMatch.home,
                        selected: false,
                        played: false,
                        isHome: playerMatch.home === player.currentClub
                    });
                }
            });
            
            player.selectedMatches = [];
        }

        function initializeLeagueTables() {
            teamsData.countries.forEach(country => {
                Object.keys(country.divisions).forEach(divNum => {
                    const divKey = `${country.name}-${divNum}`;
                    const divData = country.divisions[divNum];
                    console.log('Initializing league table for:', divKey, 'with', divData.teams.length, 'teams');
                    leagueTables[divKey] = divData.teams.map(team => ({
                        team: team.name,
                        played: 0,
                        won: 0,
                        drawn: 0,
                        lost: 0,
                        goalsFor: 0,
                        goalsAgainst: 0,
                        points: 0
                    }));
                    console.log('League table created for', divKey, ':', leagueTables[divKey]);
                });
            });
            
            initializeLeagueFixtures();
            console.log('League fixtures generated for all divisions');
        }

        function getTeamStats(divKey, teamName) {
            return leagueTables[divKey].find(t => t.team === teamName);
        }

        function simulateMatch(team1, team2, divKey) {
            const stats1 = getTeamStats(divKey, team1);
            const stats2 = getTeamStats(divKey, team2);
            
            const divData = divisions[divKey];
            const divStrength = (5 - divData.division) * 0.5;
            const baseGoals = 1 + divStrength;
            
            let goals1, goals2;
            
            if (team1 === player.currentClub) {
                const shootingBonus = player.stats.shooting / 100;
                goals1 = Math.floor(Math.random() * (baseGoals + shootingBonus * 2) + shootingBonus);
                goals2 = Math.floor(Math.random() * (baseGoals + 1));
            } else if (team2 === player.currentClub) {
                const shootingBonus = player.stats.shooting / 100;
                goals2 = Math.floor(Math.random() * (baseGoals + shootingBonus * 2) + shootingBonus);
                goals1 = Math.floor(Math.random() * (baseGoals + 1));
            } else {
                goals1 = Math.floor(Math.random() * (baseGoals + 1));
                goals2 = Math.floor(Math.random() * (baseGoals + 1));
            }
            
            stats1.played++;
            stats2.played++;
            stats1.goalsFor += goals1;
            stats1.goalsAgainst += goals2;
            stats2.goalsFor += goals2;
            stats2.goalsAgainst += goals1;
            
            if (goals1 > goals2) {
                stats1.won++;
                stats2.lost++;
                stats1.points += 3;
            } else if (goals2 > goals1) {
                stats2.won++;
                stats1.lost++;
                stats2.points += 3;
            } else {
                stats1.drawn++;
                stats2.drawn++;
                stats1.points += 1;
                stats2.points += 1;
            }
            
            return { team1, team2, goals1, goals2 };
        }

        function simulateMatchWithPlayerGoals(team1, team2, divKey, playerGoals) {
            const stats1 = getTeamStats(divKey, team1);
            const stats2 = getTeamStats(divKey, team2);

            const divData = divisions[divKey];
            const divStrength = (5 - divData.division) * 0.5;
            const opponentGoals = Math.floor(Math.random() * (2 + divStrength));

            let goals1, goals2;

            if (team1 === player.currentClub) {
                goals1 = playerGoals;
                goals2 = opponentGoals;
            } else {
                goals1 = opponentGoals;
                goals2 = playerGoals;
            }

            // update stats for both
            stats1.played++;
            stats2.played++;
            stats1.goalsFor += goals1;
            stats1.goalsAgainst += goals2;
            stats2.goalsFor += goals2;
            stats2.goalsAgainst += goals1;

            if (goals1 > goals2) {
                stats1.won++;
                stats2.lost++;
                stats1.points += 3;
            } else if (goals2 > goals1) {
                stats2.won++;
                stats1.lost++;
                stats2.points += 3;
            } else {
                stats1.drawn++;
                stats2.drawn++;
                stats1.points += 1;
                stats2.points += 1;
            }

            return { team1, team2, goals1, goals2 };
        }


        function simulateWeekMatches() {
            const divKey = getCurrentDivisionKey();
            const results = [];
            
            const fixtures = leagueFixtures[divKey];
            if (!fixtures) {
                console.error('No fixtures found for division:', divKey);
                return results;
            }
            
            const weekIndex = player.week - 1;
            if (weekIndex >= fixtures.length) {
                console.error('Week index out of bounds:', weekIndex, 'for division:', divKey);
                return results;
            }
            
            const weekFixtures = fixtures[weekIndex];
            
            weekFixtures.forEach(fixture => {
                const { home, away } = fixture;
                
                if (home === player.currentClub || away === player.currentClub) {
                    matchState.opponentName = home === player.currentClub ? away : home;
                    matchState.isHome = home === player.currentClub;

                    const playerGoals = matchState.teamScore > 0 ? matchState.teamScore : player.career.goals;
                    const result = simulateMatchWithPlayerGoals(
                        home,
                        away,
                        divKey,
                        playerGoals
                    );
                    results.push(result);

                    // Reset playerGoals after applying them
                    matchState.teamScore = 0;
                    player.career.goals = 0;
                } else {
                    const result = simulateMatch(home, away, divKey);
                    results.push(result);
                }
            });
            
            teamsData.countries.forEach(country => {
                Object.keys(country.divisions).forEach(divNum => {
                    const otherDivKey = `${country.name}-${divNum}`;
                    if (otherDivKey === divKey) return;
                    
                    const otherFixtures = leagueFixtures[otherDivKey];
                    if (!otherFixtures) return;
                    if (!leagueTables[otherDivKey]) return;
                    
                    const otherWeekFixtures = otherFixtures[weekIndex];
                    if (!otherWeekFixtures) return;
                    
                    otherWeekFixtures.forEach(fixture => {
                        simulateMatch(fixture.home, fixture.away, otherDivKey);
                    });
                });
            });
            
            return results;
        }

        function sortLeagueTable(divKey) {
            leagueTables[divKey].sort((a, b) => {
                if (b.points !== a.points) return b.points - a.points;
                const gd1 = a.goalsFor - a.goalsAgainst;
                const gd2 = b.goalsFor - b.goalsAgainst;
                if (gd2 !== gd1) return gd2 - gd1;
                return b.goalsFor - a.goalsFor;
            });
        }

        function calculateOverall() {
            const stats = player.stats;
            let overall = Math.floor((stats.speed + stats.shooting + stats.passing + 
                                       stats.defense + stats.stamina) / 5);
            return Math.min(99, Math.max(1, overall));
        }

        function calculateValue() {
            const overall = calculateOverall();
            const base = Math.floor(overall / 10);
            return Math.max(1, base + Math.floor(player.career.goals / 10));
        }

        function showScreen(screenId) {
            document.querySelectorAll('.screen').forEach(screen => {
                screen.classList.remove('active');
            });
            document.getElementById(screenId).classList.add('active');
            
            if (screenId === 'statsScreen') updateStatsDisplay();
            if (screenId === 'trainingScreen') updateTrainingDisplay();
            if (screenId === 'leagueScreen') {
                initializeLeagueSelectors();
                updateLeagueDisplay();
            }
            if (screenId === 'matchScreen') {
                resetMatchState();
                updateMatchDisplay();
            }
            if (screenId === 'matchSelectionScreen') updateMatchSelectionDisplay();
            if (screenId === 'shopScreen') updateShopDisplay();
            if (screenId === 'transferScreen') updateTransferDisplay();
            if (screenId === 'transferOfferScreen') {
                if (transferOffers.length === 0 && pendingTransferOffer === null) {
                    showScreen('transferScreen');
                }
            }
            if (screenId === 'careerScreen') updateCareerDisplay();
            if (screenId === 'cupScreen') updateCupDisplay();
            if (screenId === 'settingsScreen') updateSettingsDisplay();
            if (screenId === 'penaltyScreen') setupPenaltyGame();
        }

        function updateAllDisplays() {
            document.getElementById('menuPlayerName').textContent = player.name;
            document.getElementById('menuSeason').textContent = player.season;
            document.getElementById('menuWeek').textContent = player.week;
            document.getElementById('menuCountry').textContent = player.country;
            document.getElementById('menuDivision').textContent = player.divisionName;
            document.getElementById('menuClub').textContent = player.currentClub;
            document.getElementById('menuOverall').textContent = calculateOverall();
            document.getElementById('menuMoney').textContent = player.money;
            document.getElementById('menuSalary').textContent = player.weeklySalary;
            document.getElementById('menuGoalCards').textContent = player.goalCards;
            document.getElementById('menuIncidentCards').textContent = player.incidentCards;
            document.getElementById('menuTransferCards').textContent = player.transferCards;
            document.getElementById('menuVotes').textContent = player.career.votes;
            updateAvatarDisplay();
            updateCardManagementDisplay();
            const incidentDiv = document.getElementById('incidentDisplay');
            if (player.incidentCard) {
                incidentDiv.style.display = 'block';
                incidentDiv.innerHTML = `<p><strong>${player.incidentCard.text}</strong></p><p>${player.incidentCard.result || ''}</p>`;
                if (player.injured) {
                    incidentDiv.innerHTML += `<p style="color: #ff6666;">⚠️ Injured for ${player.weeksInjured} more week(s)!</p>`;
                }
            } else if (transferOffers.length > 0 || pendingTransferOffer) {
                incidentDiv.style.display = 'block';
                incidentDiv.innerHTML = '<p style="color: #ffd700;"><strong>You have transfer offer(s) pending!</strong></p><p>Check Transfers menu to view offers.</p>';
            } else {
                incidentDiv.style.display = 'none';
            }
        }

        function updateCardManagementDisplay() {
            document.getElementById('mgGoalCards').textContent = player.goalCards;
            document.getElementById('mgIncidentCards').textContent = player.incidentCards;
            document.getElementById('mgTransferCards').textContent = player.transferCards;
        }

        function updateStatsDisplay() {
            document.getElementById('statsPlayerName').textContent = player.name;
            document.getElementById('statOverall').textContent = calculateOverall();
            document.getElementById('statSpeed').textContent = player.stats.speed;
            document.getElementById('statShooting').textContent = player.stats.shooting;
            document.getElementById('statPassing').textContent = player.stats.passing;
            document.getElementById('statDefense').textContent = player.stats.defense;
            document.getElementById('statStamina').textContent = player.stats.stamina;
            document.getElementById('statAge').textContent = player.age;
            document.getElementById('statEnergy').textContent = player.energy;
        }

        function updateTrainingDisplay() {
            document.getElementById('trainingEnergy').textContent = player.energy;
        }

        function resetMatchState() {
            matchState = {
                teamScore: 0,
                opponentScore: 0,
                playerGoals: 0,
                playerGoalsThisMatch: 0,
                isHome: true,
                opponentName: '',
                isPlaying: false,
                isCupMatch: false,
                cupType: '',
                penaltyType: 1,
                defenderPositions: [],
                shots: 0,
                shotsOnTarget: 0,
                performanceRating: 0,
                weeklyReportData: {}
            };
        }

        function updateMatchDisplay() {
            const divKey = getCurrentDivisionKey();
            const currentDivision = divisions[divKey];
            
            document.getElementById('matchWeek').textContent = player.week;
            
            let opponent;
            let canPlay = true;
            
            if (matchState.isCupMatch) {
                let allTeams = [];
                teamsData.countries.forEach(country => {
                    Object.keys(country.divisions).forEach(divNum => {
                        if (parseInt(divNum) <= 2) {
                            country.divisions[divNum].teams.forEach(team => {
                                allTeams.push(team.name);
                            });
                        }
                    });
                });
                opponent = allTeams[Math.floor(Math.random() * allTeams.length)];
            } else {
                const fixtures = leagueFixtures[divKey];
                if (!fixtures) {
                    console.error('No fixtures found for division:', divKey);
                    opponent = 'Unknown';
                } else {
                    const weekIndex = player.week - 1;
                    if (weekIndex >= fixtures.length) {
                        console.error('Week index out of bounds:', weekIndex);
                        opponent = 'Unknown';
                    } else {
                        const weekFixtures = fixtures[weekIndex];
                        const playerMatch = weekFixtures.find(m => 
                            m.home === player.currentClub || m.away === player.currentClub
                        );
                        
                        if (playerMatch) {
                            opponent = playerMatch.home === player.currentClub ? playerMatch.away : playerMatch.home;
                            matchState.isHome = playerMatch.home === player.currentClub;
                        } else {
                            console.error('Player team not found in fixtures for week:', weekIndex);
                            opponent = 'Unknown';
                        }
                    }
                }
                
                if (!player.selectedMatches.includes(player.week - 1) && player.selectedMatches.length > 0) {
                    addMatchEvent('This match is not in your schedule. Select matches first!', 'event');
                    canPlay = false;
                }
            }
            
            matchState.opponentName = opponent;
            
            document.getElementById('homeTeam').textContent = matchState.isHome ? player.currentClub : opponent;
            document.getElementById('awayTeam').textContent = matchState.isHome ? opponent : player.currentClub;
            document.getElementById('matchGoalCards').textContent = player.goalCards;
            document.getElementById('matchResult').style.display = 'none';
            document.getElementById('matchEvents').innerHTML = '';
            document.getElementById('useCardBtn').style.display = 'inline-block';
            document.getElementById('endMatchBtn').style.display = 'none';
            document.getElementById('useCardBtn').disabled = !canPlay || player.injured;
            
            if (player.injured) {
                addMatchEvent('You are injured and cannot play!', 'event');
            }
            
            if (matchState.isPlaying) {
                document.getElementById('endMatchBtn').style.display = 'inline-block';
            }
        }

        function updateShopDisplay() {
            const divKey = getCurrentDivisionKey();
            const currentDivision = divisions[divKey];
            
            document.getElementById('shopMoney').textContent = player.money;
            document.getElementById('shopDivision').textContent = player.divisionName;
            const cardPrice = currentDivision.goalCardPrice;
            document.getElementById('shopCardPrice').textContent = cardPrice;
            
            const shopItems = document.getElementById('shopItems');
            shopItems.innerHTML = '';

            const items = [
                { name: 'Goal Card (x1)', price: cardPrice, cards: 1, desc: 'Take a penalty shot' },
                { name: 'Goal Cards (x3)', price: cardPrice * 3, cards: 3, desc: '3 penalty shots' },
                { name: 'Goal Cards (x5)', price: cardPrice * 5, cards: 5, desc: '5 penalty shots' },
                { name: 'Incident Card (x1)', price: cardPrice, incidentCards: 1, desc: 'Random event (good or bad!)' },
                { name: 'Incident Cards (x3)', price: cardPrice * 3, incidentCards: 3, desc: '3 random events' },
                { name: 'Transfer Card', price: cardPrice, transferCards: 1, desc: 'Get transfer offers' },
                { name: 'Energy Drink', price: 150, energy: 30, desc: 'Restore 30 energy' },
                { name: 'Premium Training', price: 400, statBoost: 3, desc: '+3 to all stats' }
            ];

            items.forEach((item, index) => {
                const div = document.createElement('div');
                div.className = 'shop-item';
                div.innerHTML = `
                    <div class="shop-item-info">
                        <strong>${item.name}</strong><br>
                        <small>${item.desc}</small>
                    </div>
                    <div class="shop-item-price">£${item.price}</div>
                    <button class="btn" onclick="buyItem(${index})">Buy</button>
                `;
                shopItems.appendChild(div);
            });
        }

        function buyItem(index) {
            const divKey = getCurrentDivisionKey();
            const currentDivision = divisions[divKey];
            const cardPrice = currentDivision.goalCardPrice;
            
            const items = [
                { price: cardPrice, cards: 1 },
                { price: cardPrice * 3, cards: 3 },
                { price: cardPrice * 5, cards: 5 },
                { price: cardPrice, incidentCards: 1 },
                { price: cardPrice * 3, incidentCards: 3 },
                { price: cardPrice, transferCards: 1 },
                { price: 150, energy: 30 },
                { price: 400, statBoost: 3 }
            ];

            const item = items[index];

            if (player.money < item.price) {
                alert('Not enough money!');
                return;
            }

            player.money -= item.price;

            if (item.cards) {
                player.goalCards += item.cards;
                alert(`Purchased ${item.cards} goal card(s)!`);
            } else if (item.incidentCards) {
                player.incidentCards += item.incidentCards;
                alert(`Purchased ${item.incidentCards} incident card(s)!`);
            } else if (item.transferCards) {
                player.transferCards += item.transferCards;
                alert(`Purchased ${item.transferCards} transfer card(s)!`);
            } else if (item.energy) {
                player.energy = Math.min(100, player.energy + item.energy);
                alert(`Energy restored by ${item.energy}!`);
            } else if (item.statBoost) {
                player.stats.speed = Math.min(99, player.stats.speed + item.statBoost);
                player.stats.shooting = Math.min(99, player.stats.shooting + item.statBoost);
                player.stats.passing = Math.min(99, player.stats.passing + item.statBoost);
                player.stats.defense = Math.min(99, player.stats.defense + item.statBoost);
                player.stats.stamina = Math.min(99, player.stats.stamina + item.statBoost);
                alert(`All stats increased by ${item.statBoost}!`);
            }

            player.value = calculateValue();
            updateAllDisplays();
            updateShopDisplay();
        }

        function updateTransferDisplay() {
            const divKey = getCurrentDivisionKey();
            const currentDivision = divisions[divKey];
            
            document.getElementById('currentClub').textContent = player.currentClub;
            document.getElementById('currentValue').textContent = player.value;
            
            const transferList = document.getElementById('transferList');
            transferList.innerHTML = '';
            
            const availableClubs = currentDivision.teams.filter(c => c !== player.currentClub);
            availableClubs.forEach(club => {
                const interest = Math.random();
                if (interest > 0.5) {
                    const offer = Math.max(1, player.value + Math.floor(Math.random() * 3));
                    const item = document.createElement('div');
                    item.className = 'transfer-item';
                    item.innerHTML = `
                        <strong>${club}</strong><br>
                        Offer: £${offer}M<br>
                        <small>Interest: ${Math.floor(interest * 100)}%</small>
                    `;
                    item.onclick = () => transferToClub(club, offer);
                    transferList.appendChild(item);
                }
            });
        }

        function transferToClub(club, offer) {
            if (confirm(`Transfer to ${club} for £${offer}M?`)) {
                player.currentClub = club;
                player.money += offer * 1000;
                player.value = offer;
                player.career.clubs.push(club);
                alert(`Welcome to ${club}! Transfer fee: £${offer}M`);
                updateAllDisplays();
                updateTransferDisplay();
            }
        }

        function updateCareerDisplay() {
            document.getElementById('careerPlayerName').textContent = player.name;
            document.getElementById('careerSeasons').textContent = player.season - 1;
            document.getElementById('careerGoals').textContent = player.career.totalGoalsAllTime + player.career.goals;
            document.getElementById('careerMatches').textContent = player.career.matches;
            document.getElementById('careerTrophies').textContent = player.career.trophies;
            document.getElementById('careerClubs').textContent = player.career.clubs.length;
            document.getElementById('careerVotes').textContent = player.career.votes;
            document.getElementById('careerHighestDiv').textContent = player.divisionName;
        }

        function train(type) {
            if (player.energy < 10 && type !== 'rest') {
                alert('Not enough energy! Rest first.');
                return;
            }

            switch(type) {
                case 'speed':
                    player.stats.speed = Math.min(99, player.stats.speed + 2);
                    player.energy -= 10;
                    break;
                case 'shooting':
                    player.stats.shooting = Math.min(99, player.stats.shooting + 2);
                    player.energy -= 10;
                    break;
                case 'passing':
                    player.stats.passing = Math.min(99, player.stats.passing + 2);
                    player.energy -= 10;
                    break;
                case 'defense':
                    player.stats.defense = Math.min(99, player.stats.defense + 2);
                    player.energy -= 10;
                    break;
                case 'stamina':
                    player.stats.stamina = Math.min(99, player.stats.stamina + 2);
                    player.energy -= 10;
                    break;
                case 'rest':
                    player.energy = Math.min(100, player.energy + 30);
                    break;
            }

            player.value = calculateValue();
            updateAllDisplays();
            updateTrainingDisplay();
        }

        function useGoalCard() {
            if (player.goalCards <= 0) {
                alert('No goal cards available! Visit the shop.');
                return;
            }

            if (player.injured) {
                alert('You are injured and cannot play!');
                return;
            }

            if (!matchState.isPlaying) {
                matchState.isPlaying = true;
                document.getElementById('matchResult').style.display = 'block';
                document.getElementById('finalScore').textContent = `${matchState.teamScore} - ${matchState.opponentScore}`;
            }

            player.goalCards--;
            document.getElementById('matchGoalCards').textContent = player.goalCards;
            showScreen('penaltyScreen');
        }

        function useIncidentCard() {
            if (player.incidentCards <= 0) {
                alert('No incident cards available! Visit the shop to buy more.');
                return;
            }

            player.incidentCards--;

            const totalWeight = incidentCards.reduce((sum, card) => sum + card.weight, 0);
            let randomWeight = Math.random() * totalWeight;
            let selectedIncident = incidentCards[incidentCards.length - 1];

            for (const incident of incidentCards) {
                randomWeight -= incident.weight;
                if (randomWeight <= 0) {
                    selectedIncident = incident;
                    break;
                }
            }

            const result = selectedIncident.effect();
            alert(`${selectedIncident.text}\n\nResult: ${result}`);
            player.incidentCard = selectedIncident;
            player.incidentCard.result = result;

            updateAllDisplays();
        }

        function useTransferCard() {
            if (player.transferCards <= 0) {
                alert('No transfer cards available! Visit the shop to buy more.');
                return;
            }

            player.transferCards--;
            const offers = generateTransferOffers(4, false);
            showMultipleTransferOffers(offers);
            updateAllDisplays();
        }

        function setupPenaltyGame() {
            document.getElementById('penaltyShooting').textContent = player.stats.shooting;
            document.getElementById('penaltyResult').style.display = 'none';
            document.getElementById('backToMatchBtn').style.display = 'none';

            const rand = Math.random();
            if (rand < 0.4) {
                matchState.penaltyType = 1;
            } else if (rand < 0.8) {
                matchState.penaltyType = 2;
            } else {
                matchState.penaltyType = 3;
            }

            const typeText = {
                1: 'Type 1 - No Defenders (Easy)',
                2: 'Type 2 - 1 Defender (Medium)',
                3: 'Type 3 - 2 Defenders (Hard)'
            };
            document.getElementById('penaltyType').textContent = typeText[matchState.penaltyType];

            const goalkeeper = document.getElementById('goalkeeper');
            goalkeeper.style.left = '50%';

            const defender1 = document.getElementById('defender1');
            const defender2 = document.getElementById('defender2');
            defender1.style.display = matchState.penaltyType >= 2 ? 'block' : 'none';
            defender2.style.display = matchState.penaltyType === 3 ? 'block' : 'none';

            positionDefenders();

            const ball = document.getElementById('ball');
            ball.style.bottom = '-80px';
            ball.style.left = '50%';
            ball.style.transform = 'translateX(-50%) scale(1)';
            ball.style.opacity = '1';
            ball.style.filter = 'none';

            const zones = document.querySelectorAll('.shot-zone');
            zones.forEach(zone => {
                zone.classList.remove('shot');
                zone.onclick = () => takePenaltyShot(parseInt(zone.dataset.zone));
            });
        }

        function positionDefenders() {
            matchState.defenderPositions = [];
            const zonesToBlock = [];

            if (matchState.penaltyType >= 2) {
                const numZones = 2 + Math.floor(Math.random() * 2);
                const availableZones = [0, 1, 2, 3, 4, 5, 6, 7, 8];
                for (let i = 0; i < numZones; i++) {
                    const idx = Math.floor(Math.random() * availableZones.length);
                    zonesToBlock.push(availableZones.splice(idx, 1)[0]);
                }
                matchState.defenderPositions.push({ zones: zonesToBlock });

                const zonePositions = {
                    0: { left: '15%', bottom: '70px' },
                    1: { left: '50%', bottom: '90px' },
                    2: { left: '85%', bottom: '70px' },
                    3: { left: '20%', bottom: '50px' },
                    4: { left: '50%', bottom: '40px' },
                    5: { left: '80%', bottom: '50px' },
                    6: { left: '25%', bottom: '20px' },
                    7: { left: '50%', bottom: '15px' },
                    8: { left: '75%', bottom: '20px' }
                };
                const avgZone = zonesToBlock.reduce((a, b) => a + b, 0) / zonesToBlock.length;
                const defender1 = document.getElementById('defender1');
                defender1.style.left = zonePositions[Math.round(avgZone)].left;
                defender1.style.bottom = '20px';
                defender1.style.transform = 'translateX(-50%)';
            }

            if (matchState.penaltyType === 3) {
                const numZones = 2 + Math.floor(Math.random() * 2);
                const availableZones = [0, 1, 2, 3, 4, 5, 6, 7, 8];
                const zonesToBlock2 = [];
                for (let i = 0; i < numZones; i++) {
                    const idx = Math.floor(Math.random() * availableZones.length);
                    zonesToBlock2.push(availableZones.splice(idx, 1)[0]);
                }
                matchState.defenderPositions.push({ zones: zonesToBlock2 });

                const zonePositions = {
                    0: { left: '15%', bottom: '70px' },
                    1: { left: '50%', bottom: '90px' },
                    2: { left: '85%', bottom: '70px' },
                    3: { left: '20%', bottom: '50px' },
                    4: { left: '50%', bottom: '40px' },
                    5: { left: '80%', bottom: '50px' },
                    6: { left: '25%', bottom: '20px' },
                    7: { left: '50%', bottom: '15px' },
                    8: { left: '75%', bottom: '20px' }
                };
                const avgZone = zonesToBlock2.reduce((a, b) => a + b, 0) / zonesToBlock2.length;
                const defender2 = document.getElementById('defender2');
                defender2.style.left = zonePositions[Math.round(avgZone)].left;
                defender2.style.bottom = '20px';
                defender2.style.transform = 'translateX(-50%)';
            }
        }

        function checkDefenderBlock(zone) {
            for (const defender of matchState.defenderPositions) {
                if (defender.zones.includes(zone)) {
                    return true;
                }
            }
            return false;
        }

        function takePenaltyShot(zone) {
            const shooting = player.stats.shooting;
            let successChance = shooting / 100;

            if (matchState.penaltyType === 1) {
                successChance += 0.1;
            } else if (matchState.penaltyType === 3) {
                successChance -= 0.15;
            }

            const isBlocked = checkDefenderBlock(zone);
            const ball = document.getElementById('ball');
            const resultDiv = document.getElementById('penaltyResult');
            const penaltyGame = document.getElementById('penaltyGame');

            const zones = document.querySelectorAll('.shot-zone');
            zones.forEach(z => z.classList.add('shot'));

            if (isBlocked) {
                const defender1 = document.getElementById('defender1');
                const defender2 = document.getElementById('defender2');
                if (matchState.penaltyType >= 2 && defender1.style.display !== 'none') {
                    defender1.classList.add('blocking');
                }
                if (matchState.penaltyType === 3 && defender2.style.display !== 'none') {
                    defender2.classList.add('blocking');
                }

                setTimeout(() => {
                    resultDiv.style.display = 'block';
                    resultDiv.className = 'penalty-result miss';
                    resultDiv.textContent = '🚫 BLOCKED BY DEFENDER!';
                    addMatchEvent(`Penalty blocked by defender!`, 'event');

                    document.getElementById('finalScore').textContent = `${matchState.teamScore} - ${matchState.opponentScore}`;
                    document.getElementById('useCardBtn').disabled = player.goalCards <= 0 || player.injured;
                    document.getElementById('backToMatchBtn').style.display = 'inline-block';
                    updateAllDisplays();
                }, 700);
                return;
            }

            const goalkeeper = document.getElementById('goalkeeper');

            const goalkeeperZones = {
                0: '20%', 1: '50%', 2: '80%',
                3: '25%', 4: '50%', 5: '75%',
                6: '30%', 7: '50%', 8: '70%'
            };

            const wrongZoneMap = {
                0: [2, 5, 8],
                1: [0, 2, 6, 8],
                2: [0, 3, 6],
                3: [2, 5, 8],
                4: [0, 2, 6, 8],
                5: [0, 3, 6],
                6: [2, 5, 8],
                7: [0, 2, 6, 8],
                8: [0, 3, 6]
            };

            const isGoal = Math.random() < successChance;

            const gkTargetZone = isGoal
                ? wrongZoneMap[zone][Math.floor(Math.random() * wrongZoneMap[zone].length)]
                : zone;
            goalkeeper.style.left = goalkeeperZones[gkTargetZone];

            const ballPositions = {
                0: { left: '15%', bottom: '70px' },
                1: { left: '50%', bottom: '90px' },
                2: { left: '85%', bottom: '70px' },
                3: { left: '20%', bottom: '50px' },
                4: { left: '50%', bottom: '40px' },
                5: { left: '80%', bottom: '50px' },
                6: { left: '25%', bottom: '20px' },
                7: { left: '50%', bottom: '15px' },
                8: { left: '75%', bottom: '20px' }
            };

            setTimeout(() => {
                ball.style.left = ballPositions[zone].left;
                ball.style.bottom = ballPositions[zone].bottom;
                ball.style.transform = 'translateX(-50%) scale(1.2)';
            }, 100);

            setTimeout(() => {
                resultDiv.style.display = 'block';

                if (isGoal) {
                    resultDiv.className = 'penalty-result goal';
                    resultDiv.textContent = '⚽ GOAL! ⚽ GOAL! ⚽';

                    for (let i = 0; i < 3; i++) {
                        setTimeout(() => {
                            const celebration = document.createElement('div');
                            celebration.className = 'goal-celebration';
                            celebration.textContent = '⚽';
                            celebration.style.animationDelay = `${i * 0.1}s`;
                            penaltyGame.appendChild(celebration);

                            setTimeout(() => {
                                celebration.remove();
                            }, 1200);
                        }, i * 100);
                    }

                    matchState.teamScore++;
                    matchState.playerGoals++;
                    matchState.playerGoalsThisMatch++;
                    matchState.shots++;
                    matchState.shotsOnTarget++;
                    addMatchEvent(`GOAL! ${player.name} scores from the penalty spot!`, 'goal-event');
                    player.career.goals++;
                    player.career.totalGoalsAllTime++;
                    player.value = calculateValue();

                    const rewards = {
                        1: { money: 100, votes: 1 },
                        2: { money: 150, votes: 1.5 },
                        3: { money: 250, votes: 2 }
                    };
                    const reward = rewards[matchState.penaltyType];
                    player.money += reward.money;

                    const votesPerGoal = player.selectedMatches.length > 0 ? Math.max(1, Math.floor(10 / player.selectedMatches.length)) : 1;
                    player.career.votes += Math.floor(votesPerGoal * reward.votes);
                    addMatchEvent(`+£${reward.money} and +${Math.floor(votesPerGoal * reward.votes)} votes for the goal!`, 'goal-event');
                } else {
                    matchState.shots++;
                    resultDiv.className = 'penalty-result miss';
                    const saveChance = (100 - shooting) / 100;
                    if (Math.random() < saveChance) {
                        resultDiv.textContent = '🧤 SAVED BY KEEPER!';
                        addMatchEvent(`Penalty saved by goalkeeper!`, 'event');
                    } else {
                        resultDiv.textContent = '❌ MISSED THE TARGET!';
                        addMatchEvent(`Penalty missed by ${player.name}!`, 'event');
                    }

                    ball.style.opacity = '0.5';
                    ball.style.filter = 'grayscale(100%)';
                }

                document.getElementById('finalScore').textContent = `${matchState.teamScore} - ${matchState.opponentScore}`;
                document.getElementById('useCardBtn').disabled = player.goalCards <= 0 || player.injured;
                document.getElementById('backToMatchBtn').style.display = 'inline-block';
                updateAllDisplays();
            }, 700);
        }

        function backToMatch() {
            showScreen('matchScreen');
            if (matchState.isPlaying) {
                document.getElementById('endMatchBtn').style.display = 'inline-block';
            }
        }

        function updateMatchSelectionDisplay() {
            document.getElementById('selectDivision').textContent = player.divisionName;
            
            const matchList = document.getElementById('matchList');
            matchList.innerHTML = '';
            
            matchSchedule.forEach((match, index) => {
                const isSelected = player.selectedMatches.includes(index);
                const div = document.createElement('div');
                div.className = 'shop-item';
                div.style.cursor = 'pointer';
                const homeAway = match.isHome ? '(H)' : '(A)';
                div.innerHTML = `
                    <div class="shop-item-info">
                        <strong>Week ${match.week}</strong><br>
                        ${match.opponent} ${homeAway}
                    </div>
                    <div style="color: ${isSelected ? '#00ff00' : '#666'}; font-weight: bold;">
                        ${isSelected ? '✓ Selected' : '○ Not Selected'}
                    </div>
                `;
                div.onclick = () => toggleMatchSelection(index);
                matchList.appendChild(div);
            });
        }

        function toggleMatchSelection(matchIndex) {
            const idx = player.selectedMatches.indexOf(matchIndex);
            if (idx > -1) {
                player.selectedMatches.splice(idx, 1);
            } else if (player.selectedMatches.length < 10) {
                player.selectedMatches.push(matchIndex);
            } else {
                alert('You can select up to 10 matches per season!');
            }
            updateMatchSelectionDisplay();
        }

        function confirmMatchSelection() {
            if (player.selectedMatches.length === 0) {
                alert('Please select at least 1 match!');
                return;
            }
            player.selectedMatches.sort((a, b) => a - b);
            alert(`${player.selectedMatches.length} matches selected for the season!`);
            showScreen('mainMenu');
        }

        function exitMatchAndNextWeek() {
            if (matchState.isPlaying) {
                if (!confirm('End current match first?')) {
                    return;
                }
                endMatch();
            } else {
                resetMatchState();
            }
            
            player.incidentCard = null;
            
            if (player.injured) {
                player.weeksInjured--;
                if (player.weeksInjured <= 0) {
                    player.injured = false;
                    alert('You have recovered from your injury!');
                }
            } else {
                const totalWeight = incidentCards.reduce((sum, card) => sum + card.weight, 0);
                let randomWeight = Math.random() * totalWeight;
                let selectedIncident = incidentCards[incidentCards.length - 1];
                
                for (const incident of incidentCards) {
                    randomWeight -= incident.weight;
                    if (randomWeight <= 0) {
                        selectedIncident = incident;
                        break;
                    }
                }
                
                player.incidentCard = selectedIncident;
                const result = selectedIncident.effect();
                player.incidentCard.result = result;
            }

            const weekResults = simulateWeekMatches();
            updateWeekResultsDisplay(weekResults);

            player.week++;
            player.energy = Math.min(100, player.energy + 10);
            player.money += player.weeklySalary;

            if (player.week > 10) {
                endSeason();
            }

            if (player.age > 31) {
                const isFootballerOfTheYear = checkFootballerOfTheYear();
                alert(`🏆 Your career has come to an end at age ${player.age}!\n\nFinal Stats:\nGoals: ${player.career.goals}\nMatches: ${player.career.matches}\nTrophies: ${player.career.trophies}\nVotes: ${player.career.votes}\nHighest Division: ${player.divisionName}\n\n${isFootballerOfTheYear ? '🌟 FOOTBALLER OF THE YEAR! 🌟' : ''}`);
                location.reload();
                return;
            }

            updateAllDisplays();
            autoSaveGame();
            showScreen('mainMenu');
        }

        function updateCupDisplay() {
            const faCupStatus = document.getElementById('faCupStatus');
            const euroCupStatus = document.getElementById('euroCupStatus');
            const cupActions = document.getElementById('cupActions');
            
            cupActions.innerHTML = '';

            if (player.division <= 2) {
                cupState.faCup.qualified = true;
                if (cupState.faCup.round === '') {
                    cupState.faCup.round = 'First Round';
                }
                faCupStatus.textContent = cupState.faCup.round;
                faCupStatus.style.color = '#00ff00';
                
                if (!cupState.faCup.played) {
                    const faCupBtn = document.createElement('button');
                    faCupBtn.className = 'btn';
                    faCupBtn.textContent = 'Play FA Cup Match';
                    faCupBtn.onclick = () => playCupMatch('fa');
                    cupActions.appendChild(faCupBtn);
                } else {
                    faCupStatus.textContent += ' (Completed)';
                }
            } else {
                faCupStatus.textContent = 'Not Qualified';
                faCupStatus.style.color = '#666';
            }

            if (player.division <= 1) {
                cupState.euroCup.qualified = true;
                if (cupState.euroCup.round === '') {
                    cupState.euroCup.round = 'Group Stage';
                }
                euroCupStatus.textContent = cupState.euroCup.round;
                euroCupStatus.style.color = '#00ff00';
                
                if (!cupState.euroCup.played) {
                    const euroCupBtn = document.createElement('button');
                    euroCupBtn.className = 'btn';
                    euroCupBtn.textContent = 'Play European Match';
                    euroCupBtn.onclick = () => playCupMatch('euro');
                    cupActions.appendChild(euroCupBtn);
                } else {
                    euroCupStatus.textContent += ' (Completed)';
                }
            } else {
                euroCupStatus.textContent = 'Not Qualified';
                euroCupStatus.style.color = '#666';
            }
        }

        function playCupMatch(cupType) {
            const cupName = cupType === 'fa' ? 'FA Cup' : 'European Cup';
            if (confirm(`Start ${cupName} match? This will use goal cards if available.`)) {
                showScreen('matchScreen');
                matchState.isCupMatch = true;
                matchState.cupType = cupType;
            }
        }

        function addMatchEvent(text, className = 'event') {
            const eventsDiv = document.getElementById('matchEvents');
            const event = document.createElement('div');
            event.className = className;
            event.textContent = text;
            eventsDiv.appendChild(event);
            eventsDiv.scrollTop = eventsDiv.scrollHeight;
        }

        function endMatch() {
            if (!matchState.isPlaying) {
                alert('Play the match first!');
                return;
            }

            player.career.matches++;

            const divKey = getCurrentDivisionKey();
            const divData = divisions[divKey];
            const divStrength = (5 - divData.division) * 0.5;
            const baseGoals = 1 + divStrength;
            const cupBonus = matchState.isCupMatch ? 0.5 : 0;
            
            let opponentGoals = 0;
            if (matchState.teamScore > 0) {
                opponentGoals = Math.floor(Math.random() * (baseGoals + 1) + cupBonus);
            } else {
                opponentGoals = Math.floor(Math.random() * (baseGoals + 2) + cupBonus);
            }
            
            matchState.opponentScore = opponentGoals;

            const matchOutcome = document.getElementById('matchOutcome');
            const isCup = matchState.isCupMatch;

            if (matchState.teamScore > matchState.opponentScore) {
                matchOutcome.textContent = isCup ? '🏆 Cup Match Won!' : '🎉 Victory! Well played!';
                matchOutcome.style.color = '#00ff00';
                player.money += isCup ? 500 : 300;
                updateForm('W');

                if (!isCup) {
                    updateLeagueAfterMatch(true, false, matchState.teamScore, matchState.opponentScore);
                }
                
                if (isCup) {
                    const cup = matchState.cupType === 'fa' ? cupState.faCup : cupState.euroCup;
                    const rounds = ['First Round', 'Second Round', 'Quarter Final', 'Semi Final', 'Final'];
                    const currentIdx = rounds.indexOf(cup.round);
                    if (currentIdx < rounds.length - 1) {
                        cup.round = rounds[currentIdx + 1];
                    } else if (cup.round === 'Final') {
                        player.career.trophies++;
                        cup.round = 'Champions!';
                        player.money += 2000;
                        alert(`🏆 Congratulations! You won the ${matchState.cupType === 'fa' ? 'FA Cup' : 'European Cup'}!`);
                    }
                    cup.played = true;
                }
            } else if (matchState.teamScore < matchState.opponentScore) {
                matchOutcome.textContent = isCup ? '😢 Cup Match Lost' : '😢 Defeat. Better luck next time.';
                matchOutcome.style.color = '#ff6666';
                player.money += isCup ? 0 : 100;
                updateForm('L');

                if (!isCup) {
                    updateLeagueAfterMatch(false, true, matchState.teamScore, matchState.opponentScore);
                }
                
                if (isCup) {
                    const cup = matchState.cupType === 'fa' ? cupState.faCup : cupState.euroCup;
                    cup.played = true;
                }
            } else {
                matchOutcome.textContent = '🤝 Draw. Good effort!';
                matchOutcome.style.color = '#ffff00';
                player.money += 200;
                updateForm('D');

                if (!isCup) {
                    updateLeagueAfterMatch(false, false, matchState.teamScore, matchState.opponentScore);
                }
            }

            matchOutcome.innerHTML += `<br><br>Your Goals: ${matchState.playerGoalsThisMatch}`;
            matchState.performanceRating = calculatePerformanceRating();
            document.getElementById('useCardBtn').style.display = 'none';
            document.getElementById('endMatchBtn').style.display = 'none';
            matchState.isCupMatch = false;
            updateAllDisplays();
            updateLeagueDisplay();
            autoSaveGame();
        }

        function updateLeagueAfterMatch(isWin, isLoss, goalsFor, goalsAgainst) {
            const divKey = getCurrentDivisionKey();
            const teamStats = getTeamStats(divKey, player.currentClub);
            if (!teamStats) {
                console.error('Team stats not found for:', player.currentClub);
                return;
            }
            teamStats.played++;
            teamStats.goalsFor += goalsFor;
            teamStats.goalsAgainst += goalsAgainst;
            
            if (isWin) {
                teamStats.won++;
                teamStats.points += 3;
            } else if (isLoss) {
                teamStats.lost++;
            } else {
                teamStats.drawn++;
                teamStats.points += 1;
            }
            
            const opponentStats = getTeamStats(divKey, matchState.opponentName);
            if (opponentStats) {
                opponentStats.played++;
                opponentStats.goalsFor += goalsAgainst;
                opponentStats.goalsAgainst += goalsFor;
                
                if (isLoss) {
                    opponentStats.won++;
                    opponentStats.points += 3;
                } else if (isWin) {
                    opponentStats.lost++;
                } else {
                    opponentStats.drawn++;
                    opponentStats.points += 1;
                }
            }
        }

        function calculatePlayerPerformance() {
            const totalGoals = player.career.goals;
            const matchesPlayed = player.career.matches;
            const goalsPerMatch = matchesPlayed > 0 ? totalGoals / matchesPlayed : 0;
            const overall = calculateOverall();
            const value = player.value;

            return {
                goalsPerMatch: goalsPerMatch,
                overall: overall,
                value: value,
                totalGoals: totalGoals,
                matchesPlayed: matchesPlayed
            };
        }

        function generateTransferOffers(count, isEndOfSeason) {
            if (count === undefined) count = 2;
            if (isEndOfSeason === undefined) isEndOfSeason = false;

            transferOffers = [];
            const performance = calculatePlayerPerformance();
            const currentDiv = player.division;
            const currentCountry = player.country;

            let targetDivisions = [];

            if (performance.goalsPerMatch >= 1.0 || performance.overall >= 70) {
                targetDivisions = [currentDiv - 1, currentDiv, currentDiv - 2].filter(d => d >= 1);
            } else if (performance.goalsPerMatch < 0.3 || performance.overall < 55) {
                targetDivisions = [currentDiv + 1, currentDiv].filter(d => d <= 4);
            } else {
                targetDivisions = [currentDiv - 1, currentDiv, currentDiv + 1].filter(d => d >= 1 && d <= 4);
            }

            const allCountries = teamsData.countries.map(c => c.name);
            const countriesToSearch = isEndOfSeason ? allCountries : [currentCountry];

            countriesToSearch.forEach(country => {
                targetDivisions.forEach(div => {
                    const divKey = country + '-' + div;
                    if (divisions[divKey]) {
                        const teamsInDiv = divisions[divKey].teams.filter(t => t !== player.currentClub);
                        teamsInDiv.forEach(team => {
                            const interest = Math.random();
                            if (interest > 0.6) {
                                const baseSalary = (5 - div) * 100;
                                const salaryBonus = Math.floor(performance.overall * 2);
                                const newSalary = baseSalary + salaryBonus;

                                transferOffers.push({
                                    club: team,
                                    country: country,
                                    division: div,
                                    divisionName: divisions[divKey].name,
                                    salary: newSalary,
                                    transferFee: Math.max(1, performance.value + Math.floor(Math.random() * 3)),
                                    interest: interest
                                });
                            }
                        });
                    }
                });
            });

            transferOffers.sort((a, b) => b.interest - a.interest);
            transferOffers = transferOffers.slice(0, count);

            return transferOffers;
        }

        function showTransferOffer(offer, isEndOfSeason) {
            if (isEndOfSeason === undefined) isEndOfSeason = false;

            pendingTransferOffer = offer;
            const offerList = document.getElementById('transferOfferList');
            offerList.innerHTML = '';

            const offerCard = document.createElement('div');
            offerCard.className = 'transfer-offer-card';
            offerCard.innerHTML = '<div class="transfer-offer-header">Transfer Offer</div>' +
                '<div class="transfer-offer-details">' +
                '<strong>Club:</strong> ' + offer.club + '<br>' +
                '<strong>Country:</strong> ' + offer.country + '<br>' +
                '<strong>Division:</strong> ' + offer.division + ' - ' + offer.divisionName + '<br>' +
                '<strong>Weekly Salary:</strong> ' + offer.salary + '<br>' +
                '<strong>Transfer Fee:</strong> ' + offer.transferFee + 'M' +
                '</div>' +
                '<div class="transfer-actions">' +
                '<button class="btn accept-btn" id="acceptTransferBtn">Accept</button>' +
                '<button class="btn decline-btn" id="declineTransferBtn">Decline</button>' +
                '</div>';
            offerList.appendChild(offerCard);

            document.getElementById('acceptTransferBtn').onclick = function() {
                acceptTransferOffer();
            };

            document.getElementById('declineTransferBtn').onclick = function() {
                declineTransferOffer(isEndOfSeason);
            };

            showScreen('transferOfferScreen');
        }

        function showMultipleTransferOffers(offers) {
            transferOffers = offers;
            const offerList = document.getElementById('transferOfferList');
            offerList.innerHTML = '';

            offers.forEach((offer, index) => {
                const offerCard = document.createElement('div');
                offerCard.className = 'transfer-offer-card';
                offerCard.innerHTML = '<div class="transfer-offer-header">Transfer Offer #' + (index + 1) + '</div>' +
                    '<div class="transfer-offer-details">' +
                    '<strong>Club:</strong> ' + offer.club + '<br>' +
                    '<strong>Country:</strong> ' + offer.country + '<br>' +
                    '<strong>Division:</strong> ' + offer.division + ' - ' + offer.divisionName + '<br>' +
                    '<strong>Weekly Salary:</strong> ' + offer.salary + '<br>' +
                    '<strong>Transfer Fee:</strong> ' + offer.transferFee + 'M' +
                    '</div>' +
                    '<div class="transfer-actions">' +
                    '<button class="btn accept-btn" id="acceptTransferBtn' + index + '">Accept</button>' +
                    '<button class="btn decline-btn" id="declineTransferBtn' + index + '">Decline</button>' +
                    '</div>';
                offerList.appendChild(offerCard);

                document.getElementById('acceptTransferBtn' + index).onclick = function() {
                    acceptSpecificTransferOffer(index);
                };

                document.getElementById('declineTransferBtn' + index).onclick = function() {
                    declineSpecificTransferOffer(index);
                };
            });

            if (offers.length === 0) {
                offerList.innerHTML = '<div class="no-offers">No transfer offers available at this time.</div>';
            }

            showScreen('transferOfferScreen');
        }

        function acceptTransferOffer() {
            if (pendingTransferOffer) {
                player.currentClub = pendingTransferOffer.club;
                player.country = pendingTransferOffer.country;
                player.division = pendingTransferOffer.division;
                player.divisionName = pendingTransferOffer.division + ' - ' + pendingTransferOffer.divisionName;
                player.weeklySalary = pendingTransferOffer.salary;
                player.career.clubs.push(pendingTransferOffer.club);
                player.value = pendingTransferOffer.transferFee;
                player.money = player.money + (pendingTransferOffer.transferFee * 1000);

                alert('Welcome to ' + pendingTransferOffer.club + '!\n\nYour new weekly salary is ' + pendingTransferOffer.salary + '\nTransfer fee: ' + pendingTransferOffer.transferFee + 'M');

                pendingTransferOffer = null;
                midSeasonOfferReceived = false;

                leagueView.country = null;
                leagueView.division = null;

                updateAllDisplays();
                initializeDivisions();
                initializeLeagueFixtures();
                initializeLeagueTables();
                generateMatchSchedule();
                showScreen('mainMenu');
            }
        }

        function acceptSpecificTransferOffer(index) {
            const offer = transferOffers[index];
            player.currentClub = offer.club;
            player.country = offer.country;
            player.division = offer.division;
            player.divisionName = offer.division + ' - ' + offer.divisionName;
            player.weeklySalary = offer.salary;
            player.career.clubs.push(offer.club);
            player.value = offer.transferFee;
            player.money = player.money + (offer.transferFee * 1000);

            alert('Welcome to ' + offer.club + '!\n\nYour new weekly salary is ' + offer.salary + '\nTransfer fee: ' + offer.transferFee + 'M');

            transferOffers = [];

            leagueView.country = null;
            leagueView.division = null;

            showScreen('mainMenu');
            updateAllDisplays();
            initializeDivisions();
            initializeLeagueFixtures();
            initializeLeagueTables();
            generateMatchSchedule();
        }

        function declineTransferOffer(isEndOfSeason) {
            pendingTransferOffer = null;
            if (isEndOfSeason) {
                alert('Transfer offer declined.');
            }
            showScreen('mainMenu');
        }

        function declineSpecificTransferOffer(index) {
            transferOffers.splice(index, 1);

            if (transferOffers.length > 0) {
                showMultipleTransferOffers(transferOffers);
            } else {
                alert('All transfer offers declined.');
                showScreen('mainMenu');
            }
        }

        function closeTransferOffers() {
            transferOffers = [];
            pendingTransferOffer = null;
            showScreen('mainMenu');
        }

        function checkTransferOffers() {
            if (transferOffers.length > 0 || pendingTransferOffer) {
                showScreen('transferOfferScreen');
            } else {
                const offers = generateTransferOffers(1, false);
                if (offers.length > 0) {
                    showTransferOffer(offers[0], false);
                } else {
                    showScreen('transferScreen');
                }
            }
        }

        function calculatePerformanceRating() {
            let rating = 5;

            if (matchState.playerGoalsThisMatch > 0) {
                rating += 2;
                if (matchState.playerGoalsThisMatch >= 2) rating += 1;
                if (matchState.playerGoalsThisMatch >= 3) rating += 1;
            }

            if (matchState.teamScore > matchState.opponentScore) {
                rating += 1;
            } else if (matchState.teamScore < matchState.opponentScore) {
                rating -= 1;
            }

            if (matchState.shots > 0) {
                const accuracy = matchState.shotsOnTarget / matchState.shots;
                if (accuracy >= 0.8) rating += 1;
                else if (accuracy >= 0.6) rating += 0.5;
                else if (accuracy < 0.4) rating -= 0.5;
            }

            return Math.max(1, Math.min(10, Math.round(rating * 10) / 10));
        }

        function updateForm(matchResult) {
            player.form.push(matchResult);
            if (player.form.length > 5) {
                player.form.shift();
            }
        }

        function getFormDisplay() {
            let html = '';
            for (const result of player.form) {
                const className = result === 'W' ? 'form-badge win' : (result === 'D' ? 'form-badge draw' : 'form-badge loss');
                html += `<span class="${className}">${result}</span>`;
            }
            return html;
        }

        function showWeeklyReport() {
            const oldValue = player.value;
            player.value = calculateValue();
            const newValue = player.value;
            const valueChange = newValue - oldValue;

            const divKey = getCurrentDivisionKey();
            const table = leagueTables[divKey];
            const teamStats = table ? table.find(t => t.team === player.currentClub) : null;

            let position = '-';
            let positionChange = '-';
            let points = 0;
            let gd = 0;

            if (teamStats && table) {
                const sortedTable = [...table].sort((a, b) => {
                    if (b.points !== a.points) return b.points - a.points;
                    const gd1 = a.goalsFor - a.goalsAgainst;
                    const gd2 = b.goalsFor - b.goalsAgainst;
                    if (gd2 !== gd1) return gd2 - gd1;
                    return b.goalsFor - a.goalsFor;
                });
                position = sortedTable.findIndex(t => t.team === player.currentClub) + 1;
                points = teamStats.points;
                gd = teamStats.goalsFor - teamStats.goalsAgainst;

                if (player.form.length > 0) {
                    const prevResult = player.form[player.form.length - 1];
                    if (prevResult === 'W') {
                        positionChange = '↑';
                    } else if (prevResult === 'L') {
                        positionChange = '↓';
                    } else {
                        positionChange = '-';
                    }
                }
            }

            document.getElementById('reportWeek').textContent = player.week;
            document.getElementById('reportOpponent').textContent = matchState.opponentName || '-';
            document.getElementById('reportResult').textContent = matchState.teamScore + ' - ' + matchState.opponentScore;
            document.getElementById('reportRating').textContent = matchState.performanceRating;
            document.getElementById('reportGoals').textContent = matchState.playerGoalsThisMatch;
            document.getElementById('reportShots').textContent = matchState.shots;
            document.getElementById('reportOnTarget').textContent = matchState.shotsOnTarget;
            document.getElementById('formDisplay').innerHTML = getFormDisplay();
            document.getElementById('reportPosition').textContent = position;
            document.getElementById('reportPositionChange').textContent = positionChange;
            document.getElementById('reportPoints').textContent = points;
            document.getElementById('reportGD').textContent = gd > 0 ? '+' + gd : gd;
            document.getElementById('reportOldValue').textContent = oldValue.toFixed(1);
            document.getElementById('reportNewValue').textContent = newValue.toFixed(1);
            document.getElementById('reportValueChange').textContent = valueChange >= 0 ? '+£' + valueChange.toFixed(1) + 'M' : '£' + valueChange.toFixed(1) + 'M';
            document.getElementById('reportValueChange').style.color = valueChange >= 0 ? '#00ff00' : '#ff6666';

            if (player.incidentCard && player.incidentCard.text) {
                document.getElementById('reportIncident').innerHTML = `<strong>${player.incidentCard.text}</strong><br>${player.incidentCard.result || ''}`;
            } else {
                document.getElementById('reportIncident').textContent = 'No incidents this week';
            }

            showScreen('weeklyReportScreen');
        }


        function nextWeek() {
            if (matchState.isPlaying) {
                if (!confirm('End current match first?')) {
                    return;
                }
                endMatch();
            } else {
                resetMatchState();
            }

            player.incidentCard = null;
            
            if (player.injured) {
                player.weeksInjured--;
                if (player.weeksInjured <= 0) {
                    player.injured = false;
                    alert('You have recovered from your injury!');
                }
            } else {
                const totalWeight = incidentCards.reduce((sum, card) => sum + card.weight, 0);
                let randomWeight = Math.random() * totalWeight;
                let selectedIncident = incidentCards[incidentCards.length - 1];
                
                for (const incident of incidentCards) {
                    randomWeight -= incident.weight;
                    if (randomWeight <= 0) {
                        selectedIncident = incident;
                        break;
                    }
                }
                
                player.incidentCard = selectedIncident;
                const result = selectedIncident.effect();
                player.incidentCard.result = result;
            }

            const weekResults = simulateWeekMatches();
            updateWeekResultsDisplay(weekResults);

            player.week++;
            player.energy = Math.min(100, player.energy + 10);
            player.money += player.weeklySalary;

            if (player.week > 10) {
                endSeason();
            }

            if (player.age > 31) {
                const isFootballerOfTheYear = checkFootballerOfTheYear();
                alert(`🏆 Your career has come to an end at age ${player.age}!\n\nFinal Stats:\nGoals: ${player.career.goals}\nMatches: ${player.career.matches}\nTrophies: ${player.career.trophies}\nVotes: ${player.career.votes}\nHighest Division: ${player.divisionName}\n\n${isFootballerOfTheYear ? '🌟 FOOTBALLER OF THE YEAR! 🌟' : ''}`);
                location.reload();
                return;
            }

            updateAllDisplays();
            updateLeagueDisplay();
            autoSaveGame();

            if (!player.quickMatch) {
                showWeeklyReport();
            }
        }

        function endSeason() {
            const seasonGoals = player.career.goals;
            const seasonVotes = player.career.votes;
            player.career.totalGoalsAllTime += seasonGoals;

            let teamPromoted = false;
            let teamRelegated = false;

            teamsData.countries.forEach(country => {
                Object.keys(country.divisions).forEach(divNum => {
                    const divKey = country.name + '-' + divNum;
                    if (!leagueTables[divKey]) return;

                    const sortedTable = [...leagueTables[divKey]].sort((a, b) => {
                        if (b.points !== a.points) return b.points - a.points;
                        const gd1 = a.goalsFor - a.goalsAgainst;
                        const gd2 = b.goalsFor - b.goalsAgainst;
                        if (gd2 !== gd1) return gd2 - gd1;
                        return b.goalsFor - a.goalsFor;
                    });

                    const promotedTeams = sortedTable.slice(0, 2).map(t => t.team);
                    const relegatedTeams = sortedTable.slice(-2).map(t => t.team);

                    const nextDivNum = parseInt(divNum) + 1;
                    const prevDivNum = parseInt(divNum) - 1;

                    if (nextDivNum <= 4) {
                        const nextDivKey = country.name + '-' + nextDivNum;
                        
                        relegatedTeams.forEach(teamName => {
                            const teamIndex = country.divisions[divNum].teams.findIndex(t => t.name === teamName);
                            if (teamIndex !== -1) {
                                const teamData = country.divisions[divNum].teams[teamIndex];
                                country.divisions[divNum].teams.splice(teamIndex, 1);
                                country.divisions[nextDivNum].teams.push(teamData);
                            }
                        });

                        const nextDivPromotedTeams = [...leagueTables[nextDivKey]].sort((a, b) => {
                            if (b.points !== a.points) return b.points - a.points;
                            const gd1 = a.goalsFor - a.goalsAgainst;
                            const gd2 = b.goalsFor - b.goalsAgainst;
                            if (gd2 !== gd1) return gd2 - gd1;
                            return b.goalsFor - a.goalsFor;
                        }).slice(0, 2).map(t => t.team);

                        nextDivPromotedTeams.forEach(teamName => {
                            const teamIndex = country.divisions[nextDivNum].teams.findIndex(t => t.name === teamName);
                            if (teamIndex !== -1) {
                                const teamData = country.divisions[nextDivNum].teams[teamIndex];
                                country.divisions[nextDivNum].teams.splice(teamIndex, 1);
                                country.divisions[divNum].teams.push(teamData);
                            }
                        });
                    }

                    if (prevDivNum >= 1) {
                        const prevDivKey = country.name + '-' + prevDivNum;
                        
                        promotedTeams.forEach(teamName => {
                            const teamIndex = country.divisions[divNum].teams.findIndex(t => t.name === teamName);
                            if (teamIndex !== -1) {
                                const teamData = country.divisions[divNum].teams[teamIndex];
                                country.divisions[divNum].teams.splice(teamIndex, 1);
                                country.divisions[prevDivNum].teams.push(teamData);
                            }
                        });

                        const prevDivRelegatedTeams = [...leagueTables[prevDivKey]].sort((a, b) => {
                            if (b.points !== a.points) return b.points - a.points;
                            const gd1 = a.goalsFor - a.goalsAgainst;
                            const gd2 = b.goalsFor - b.goalsAgainst;
                            if (gd2 !== gd1) return gd2 - gd1;
                            return b.goalsFor - a.goalsFor;
                        }).slice(-2).map(t => t.team);

                        prevDivRelegatedTeams.forEach(teamName => {
                            const teamIndex = country.divisions[prevDivNum].teams.findIndex(t => t.name === teamName);
                            if (teamIndex !== -1) {
                                const teamData = country.divisions[prevDivNum].teams[teamIndex];
                                country.divisions[prevDivNum].teams.splice(teamIndex, 1);
                                country.divisions[divNum].teams.push(teamData);
                            }
                        });
                    }
                    if (promotedTeams.includes(player.currentClub) && player.country === country.name) {
                        teamPromoted = true;
                        player.division = Math.max(1, parseInt(divNum) - 1); // prevent 0
                        const countryData = teamsData.countries.find(c => c.name === player.country);
                        const newDivData = countryData.divisions[player.division];
                        if (newDivData) {
                            player.divisionName = player.division + ' - ' + newDivData.name;
                        }
                        player.weeklySalary = player.weeklySalary + 50;
                    }

                    if (relegatedTeams.includes(player.currentClub) && player.country === country.name) {
                        teamRelegated = true;
                        player.division = Math.min(4, parseInt(divNum) + 1); // prevent 5
                        const countryData = teamsData.countries.find(c => c.name === player.country);
                        const newDivData = countryData.divisions[player.division];
                        if (newDivData) {
                            player.divisionName = player.division + ' - ' + newDivData.name;
                        }
                        player.weeklySalary = Math.max(50, player.weeklySalary - 30);
                    }

                });
            });

            let seasonSummary = 'Season ' + (player.season - 1) + ' Complete!\\n\\n';
            seasonSummary += 'Goals Scored This Season: ' + seasonGoals + '\\n';
            seasonSummary += 'Votes Earned This Season: ' + seasonVotes + '\\n';
            seasonSummary += 'Total Career Goals: ' + player.career.totalGoalsAllTime + '\\n';
            seasonSummary += 'Total Career Votes: ' + player.career.votes + '\\n';

            if (teamPromoted) {
                seasonSummary += '\\n🎉 ' + player.currentClub + ' has been PROMOTED to ' + player.divisionName + '!\\n';
                seasonSummary += 'Your salary is now £' + player.weeklySalary + '/week';
            } else if (teamRelegated) {
                seasonSummary += '\\n😔 ' + player.currentClub + ' has been RELEGATED to ' + player.divisionName + '\\n';
                seasonSummary += 'Your salary is now £' + player.weeklySalary + '/week';
            } else {
                seasonSummary += '\\n' + player.currentClub + ' remains in ' + player.divisionName;
            }

            alert(seasonSummary);

            player.career.goals = 0;
            player.week = 1;
            player.season = player.season + 1;
            player.age = player.age + 1;
            midSeasonOfferReceived = false;

            initializeDivisions();
            initializeLeagueFixtures();
            initializeLeagueTables();
            generateMatchSchedule();
            leagueView = { country: null, division: null };

            cupState.faCup = { qualified: false, round: '', opponent: '', played: false };
            cupState.euroCup = { qualified: false, round: '', opponent: '', played: false };

            const endSeasonOffers = generateTransferOffers(2, true);
            if (endSeasonOffers.length > 0) {
                setTimeout(function() {
                    showMultipleTransferOffers(endSeasonOffers);
                }, 500);
            } else {
                showScreen('mainMenu');
            }

            updateAllDisplays();
            updateLeagueDisplay();
        }
        function checkFootballerOfTheYear() {
            const totalGoals = player.career.totalGoalsAllTime + player.career.goals;
            const avgGoalsPerSeason = totalGoals / Math.max(1, player.season - 1);
            const avgVotesPerSeason = player.career.votes / Math.max(1, player.season - 1);
            
            return avgGoalsPerSeason >= 15 && avgVotesPerSeason >= 30 && player.division <= 2;
        }

        function initializeLeagueSelectors() {
            const countrySelect = document.getElementById('leagueCountrySelect');
            const divisionSelect = document.getElementById('leagueDivisionSelect');

            countrySelect.innerHTML = '<option value="">-- Select Country --</option>';
            divisionSelect.innerHTML = '<option value="">-- Select Division --</option>';

            teamsData.countries.forEach(country => {
                const option = document.createElement('option');
                option.value = country.name;
                option.textContent = country.name;
                countrySelect.appendChild(option);
            });

            if (leagueView.country === null) {
                countrySelect.value = player.country;
            } else {
                countrySelect.value = leagueView.country;
            }

            const selectedCountry = countrySelect.value;

            divisionSelect.innerHTML = '<option value="">-- Select Division --</option>';

            if (selectedCountry) {
                const countryData = teamsData.countries.find(c => c.name === selectedCountry);
                if (countryData) {
                    for (let i = 1; i <= 4; i++) {
                        if (countryData.divisions[i]) {
                            const option = document.createElement('option');
                            option.value = i;
                            option.textContent = `${i} - ${countryData.divisions[i].name}`;
                            divisionSelect.appendChild(option);
                        }
                    }
                }
            }

            if (leagueView.division === null) {
                divisionSelect.value = player.division;
            } else {
                divisionSelect.value = leagueView.division;
            }

            updateLeagueDisplay();
        }

        function onLeagueCountryChange() {
            const countrySelect = document.getElementById('leagueCountrySelect');
            const divisionSelect = document.getElementById('leagueDivisionSelect');
            const selectedCountry = countrySelect.value;
            
            divisionSelect.innerHTML = '<option value="">-- Select Division --</option>';
            
            if (selectedCountry) {
                const countryData = teamsData.countries.find(c => c.name === selectedCountry);
                if (countryData) {
                    for (let i = 1; i <= 4; i++) {
                        if (countryData.divisions[i]) {
                            const option = document.createElement('option');
                            option.value = i;
                            option.textContent = `${i} - ${countryData.divisions[i].name}`;
                            divisionSelect.appendChild(option);
                        }
                    }
                }
                
                leagueView.country = selectedCountry;
                leagueView.division = divisionSelect.value || 1;
            }
            
            updateLeagueDisplay();
        }

        function onLeagueDivisionChange() {
            const divisionSelect = document.getElementById('leagueDivisionSelect');
            leagueView.division = parseInt(divisionSelect.value);
            updateLeagueDisplay();
        }

        function returnToMyDivision() {
            leagueView.country = null;
            leagueView.division = null;
            initializeLeagueSelectors();
        }

        function updateLeagueDisplay() {
            console.log('=== updateLeagueDisplay called ===');
            
            const leagueDivisionEl = document.getElementById('leagueDivision');
            if (!leagueDivisionEl) {
                console.log('leagueDivision element not found');
                return;
            }
            
            const countryToView = leagueView.country || player.country;
            const divisionToView = leagueView.division || player.division;
            const divKey = `${countryToView}-${divisionToView}`;
            
            console.log('Viewing division:', divKey);
            console.log('leagueView:', leagueView);
            console.log('player country/division:', player.country, '/', player.division);
            
            if (!leagueTables[divKey]) {
                console.log('ERROR: League table not found for:', divKey);
                console.log('Available keys:', Object.keys(leagueTables));
                leagueDivisionEl.textContent = 'Division not found';
                return;
            }
            
            const divData = divisions[divKey];
            console.log('divData:', divData);
            
            const countryData = teamsData.countries.find(c => c.name === countryToView);
            const divisionName = countryData ? countryData.divisions[divisionToView].name : 'Unknown';
            
            console.log('Setting header to:', `${countryToView} - Division ${divisionToView} - ${divisionName} (Week ${player.week})`);
            leagueDivisionEl.textContent = `${countryToView} - Division ${divisionToView} - ${divisionName} (Week ${player.week})`;
            
            sortLeagueTable(divKey);
            
            const tbody = document.getElementById('leagueTableBody');
            tbody.innerHTML = '';
            
            console.log('Displaying league table for:', divKey, 'with', leagueTables[divKey].length, 'teams');
            console.log('First team data:', leagueTables[divKey][0]);
            
            leagueTables[divKey].forEach((team, index) => {
                console.log(`Team ${index}:`, team.team, 'P:', team.played, 'W:', team.won, 'D:', team.drawn, 'L:', team.lost, 'Pts:', team.points);
                
                const row = document.createElement('tr');
                if (team.team === player.currentClub && countryToView === player.country && divisionToView === player.division) {
                    row.style.background = 'rgba(255, 255, 0, 0.2)';
                }
                
                const posColors = ['#ffd700', '#c0c0c0', '#cd7f32', '#fff'];
                if (index < 3) {
                    row.style.borderLeft = `4px solid ${posColors[index]}`;
                } else if (index >= divData.teams.length - 2) {
                    row.style.borderLeft = '4px solid #ff0000';
                }
                
                row.innerHTML = `
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${index + 1}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: left;">${team.team}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.played}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.won}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.drawn}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.lost}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.goalsFor}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.goalsAgainst}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center;">${team.goalsFor - team.goalsAgainst}</td>
                    <td style="padding: 8px; border: 1px solid #00ff00; text-align: center; font-weight: bold; color: #00ff00;">${team.points}</td>
                `;
                tbody.appendChild(row);
            });
            
            console.log('=== updateLeagueDisplay complete ===');
        }

        function updateWeekResultsDisplay(results) {
            const resultsDiv = document.getElementById('weekResults');
            if (results && results.length > 0) {
                resultsDiv.innerHTML = `<h3 style="color: #00ff00; margin-bottom: 10px;">Week ${player.week - 1} Results</h3>`;
                results.forEach(match => {
                    const resultText = document.createElement('p');
                    resultText.style.margin = '5px 0';
                    resultText.style.color = '#fff';
                    
                    if (match.team1 === player.currentClub || match.team2 === player.currentClub) {
                        resultText.style.background = 'rgba(255, 255, 0, 0.2)';
                        resultText.style.padding = '5px';
                        resultText.style.borderRadius = '3px';
                    }
                    
                    resultText.textContent = `${match.team1} ${match.goals1} - ${match.goals2} ${match.team2}`;
                    resultsDiv.appendChild(resultText);
                });
            } else {
                resultsDiv.innerHTML = '';
            }
        }

        function updateSettingsDisplay() {
            const hasSave = localStorage.getItem('footballerSave') !== null;
            const saveStatus = document.getElementById('saveStatus');
            if (hasSave) {
                saveStatus.textContent = '✓ Saved game found';
                saveStatus.style.color = '#00ff00';
            } else {
                saveStatus.textContent = '✗ No saved game found';
                saveStatus.style.color = '#666';
            }
        }

        function autoSaveGame() {
            const saveData = {
                player: player,
                matchState: matchState,
                matchSchedule: matchSchedule,
                cupState: cupState,
                leagueTables: leagueTables,
                leagueFixtures: leagueFixtures
            };
            localStorage.setItem('footballerSave', JSON.stringify(saveData));
        }

        function setAvatar(emoji) {
            player.avatar = emoji;
            player.avatarUrl = '';
            updateAvatarDisplay();
            autoSaveGame();
        }

        function updateAvatar() {
            const url = document.getElementById('avatarUrlInput').value;
            if (url && url.trim() !== '') {
                player.avatarUrl = url.trim();
            }
            updateAvatarDisplay();
            autoSaveGame();
        }

        function updateAvatarDisplay() {
            const menuAvatar = document.getElementById('menuAvatar');
            const settingsAvatar = document.getElementById('settingsAvatar');

            if (player.avatarUrl) {
                menuAvatar.innerHTML = `<img src="${player.avatarUrl}" style="width: 60px; height: 60px; border-radius: 50%; object-fit: cover;">`;
                settingsAvatar.innerHTML = `<img src="${player.avatarUrl}" style="width: 60px; height: 60px; border-radius: 50%; object-fit: cover;">`;
            } else {
                menuAvatar.textContent = player.avatar;
                settingsAvatar.textContent = player.avatar;
            }
        }

        function toggleQuickMatch() {
            player.quickMatch = !player.quickMatch;
            const btn = document.getElementById('quickMatchBtn');
            btn.textContent = `Currently: ${player.quickMatch ? 'ON' : 'OFF'}`;
            autoSaveGame();
        }

        function saveGame() {
            const saveData = {
                player: player,
                matchState: matchState,
                matchSchedule: matchSchedule,
                cupState: cupState,
                leagueTables: leagueTables,
                leagueFixtures: leagueFixtures
            };
            localStorage.setItem('footballerSave', JSON.stringify(saveData));
            alert('Game saved successfully!');
        }

        function loadGame() {
            const savedData = localStorage.getItem('footballerSave');
            if (!savedData) {
                alert('No saved game found!');
                return;
            }
            
            if (!confirm('Load saved game? Current progress will be lost!')) {
                return;
            }
            
            const data = JSON.parse(savedData);
            Object.assign(player, data.player);
            Object.assign(matchState, data.matchState);
            matchSchedule = data.matchSchedule;
            Object.assign(cupState, data.cupState);
            leagueTables = data.leagueTables;
            leagueFixtures = data.leagueFixtures || {};
            
            if (!leagueFixtures || Object.keys(leagueFixtures).length === 0) {
                initializeLeagueFixtures();
                console.log('Regenerated league fixtures on load');
            }
            
            updateAllDisplays();
            alert('Game loaded successfully!');
        }

        function exportGame() {
            const saveData = {
                player: player,
                matchState: matchState,
                matchSchedule: matchSchedule,
                cupState: cupState,
                leagueTables: leagueTables,
                leagueFixtures: leagueFixtures
            };
            const blob = new Blob([JSON.stringify(saveData, null, 2)], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `footballer_save_${new Date().toISOString().split('T')[0]}.json`;
            a.click();
            URL.revokeObjectURL(url);
        }

        function importGame(event) {
            const file = event.target.files[0];
            if (!file) return;
            
            const reader = new FileReader();
            reader.onload = function(e) {
                try {
                    const data = JSON.parse(e.target.result);
                    if (!data.player) {
                        alert('Invalid save file!');
                        return;
                    }
                    
                    if (!confirm('Import save file? Current progress will be lost!')) {
                        return;
                    }
                    
                    Object.assign(player, data.player);
                    Object.assign(matchState, data.matchState);
                    matchSchedule = data.matchSchedule;
                    Object.assign(cupState, data.cupState);
                    leagueTables = data.leagueTables;
                    leagueFixtures = data.leagueFixtures || {};
                    
                    if (!leagueFixtures || Object.keys(leagueFixtures).length === 0) {
                        initializeLeagueFixtures();
                        console.log('Regenerated league fixtures on import');
                    }
                    
                    updateAllDisplays();
                    alert('Game imported successfully!');
                } catch (error) {
                    alert('Error importing file: ' + error.message);
                }
            };
            reader.readAsText(file);
            event.target.value = '';
        }

        function resetGame() {
            if (!confirm('Are you sure you want to reset game? ALL progress will be lost!')) {
                return;
            }
            localStorage.removeItem('footballerSave');
            location.reload();
        }

        window.onload = function() {
            const savedGame = localStorage.getItem('footballerSave');
            if (savedGame) {
                console.log('Found saved game');
            }
        };
    </script>
</body>
</html>
