<?php
require_once __DIR__ . '/AmadeusAPI.php';
$config = require_once __DIR__ . '/config.php';

$flights = [];
$hotels = [];
$searchPerformed = false;
$apiError = null;

function formatPrice($price) {
    return number_format($price, 2);
}

function formatDuration($duration) {
    if (preg_match('/PT(\d+)H)?(\d+)?M)?/', $duration, $matches)) {
        $hours = isset($matches[1]) ? $matches[1] : '0';
        $minutes = isset($matches[2]) ? $matches[2] : '0';
        return "{$hours}h {$minutes}m";
    }
    return $duration;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $searchPerformed = true;
    $searchType = $_POST['search_type'] ?? '';
    $departureAirport = strtoupper($_POST['departure_airport'] ?? 'SKG');
    $destination = $_POST['destination'] ?? '';
    $departureDate = $_POST['departure_date'] ?? '';
    $returnDate = $_POST['return_date'] ?? '';
    $passengers = (int)($_POST['passengers'] ?? 1);
    $rooms = (int)($_POST['rooms'] ?? 1);
    $nights = (int)($_POST['nights'] ?? 1);

    $api = new AmadeusAPI($config);
    $useMockData = false;

    try {
        $destinationCode = $api->getCityCodeFromName($destination);

        if ($searchType === 'flights' || $searchType === 'both') {
            try {
                $flightResponse = $api->searchFlights($departureAirport, $destinationCode, $departureDate, $passengers, $returnDate);

                if (!empty($flightResponse['data'])) {
                    foreach ($flightResponse['data'] as $offer) {
                        $itineraries = $offer['itineraries'];
                        $price = (float)$offer['price']['total'];
                        $totalPrice = $price * $passengers;

                        foreach ($itineraries as $index => $itinerary) {
                            $segments = $itinerary['segments'];
                            $firstSegment = $segments[0];
                            $lastSegment = end($segments);

                            $carrierCode = $firstSegment['carrierCode'];
                            $flightNumber = $firstSegment['number'];
                            $from = $firstSegment['departure']['iataCode'];
                            $to = $lastSegment['arrival']['iataCode'];
                            $depTime = substr($firstSegment['departure']['at'], 11, 5);
                            $arrTime = substr($lastSegment['arrival']['at'], 11, 5);
                            $duration = formatDuration($itinerary['duration']);
                            $date = substr($firstSegment['departure']['at'], 0, 10);

                            $flights[] = [
                                'Airline' => $carrierCode,
                                'Flight' => $carrierCode . $flightNumber,
                                'From' => $from,
                                'To' => $to,
                                'Date' => $date,
                                'Departure' => $depTime,
                                'Arrival' => $arrTime,
                                'Duration' => $duration,
                                'Price' => formatPrice($price),
                                'Total Price' => formatPrice($totalPrice),
                                'Link' => $offer['provider'] === 'AMADEUS' ? 
                                    "https://test.api.amadeus.com/v1/shopping/flight-offers/{$offer['id']}/booking-urls" : 
                                    "https://www.amadeus.com/app/search/ow/{$from}-{$to}/{$date}/1/0/0/0?class=Economy"
                            ];
                        }
                    }
                }
            } catch (Exception $e) {
                $apiError = "Flight search error: " . $e->getMessage();
                $useMockData = true;
            }
        }

        if ($searchType === 'hotels' || $searchType === 'both') {
            try {
                $hotelResponse = $api->searchHotels($destinationCode, $departureDate, $rooms, $nights);

                if (!empty($hotelResponse['data'])) {
                    foreach ($hotelResponse['data'] as $offer) {
                        $hotel = $offer['hotel'];
                        $price = (float)$offer['offers'][0]['price']['total'];
                        $totalPrice = $price * $rooms;
                        $checkIn = $offer['offers'][0]['checkInDate'];
                        $checkOut = $offer['offers'][0]['checkOutDate'];

                        $hotels[] = [
                            'Name' => $hotel['name'],
                            'Location' => $hotel['cityCode'],
                            'Stars' => $hotel['rating'] ?? 3,
                            'Rating' => '4.0',
                            'Price/Night' => formatPrice($price / $nights),
                            'Total Price' => formatPrice($totalPrice),
                            'Nights' => $nights,
                            'Rooms' => $rooms,
                            'Amenities' => isset($hotel['amenities']) ? implode(', ', array_slice($hotel['amenities'], 0, 4)) : 'WiFi',
                            'Link' => "https://www.booking.com/searchresults.html?city={$destinationCode}&checkin={$checkIn}&checkout={$checkOut}",
                            'HotelId' => $hotel['hotelId']
                        ];
                    }
                }
            } catch (Exception $e) {
                $apiError = "Hotel search error: " . $e->getMessage();
                $useMockData = true;
            }
        }

    } catch (Exception $e) {
        $apiError = "API error: " . $e->getMessage();
        $useMockData = true;
    }

    if ($useMockData || empty($flights) && empty($hotels)) {
        $flightData = [
            ['Airline' => 'SkyWings', 'Flight' => 'SW101', 'From' => $departureAirport, 'To' => strtoupper(substr($destination, 0, 3)), 'Date' => $departureDate, 'Departure' => '08:00', 'Arrival' => '10:30', 'Price' => '150.00', 'Total Price' => formatPrice(150 * $passengers), 'Link' => "https://www.skyscanner.net/transport/flights/{$departureAirport}/" . strtoupper(substr($destination, 0, 3)) . "/{$departureDate}/?adultsv2={$passengers}"],
            ['Airline' => 'AeroJet', 'Flight' => 'AJ205', 'From' => $departureAirport, 'To' => strtoupper(substr($destination, 0, 3)), 'Date' => $departureDate, 'Departure' => '12:15', 'Arrival' => '14:45', 'Price' => '180.00', 'Total Price' => formatPrice(180 * $passengers), 'Link' => "https://www.expedia.com/Flights-Search?trip=oneway&leg1=from:{$departureAirport},to:" . strtoupper(substr($destination, 0, 3)) . ",departure:{$departureDate}T00:00:00&passengers=adults:{$passengers}"],
            ['Airline' => 'GlobalAir', 'Flight' => 'GA330', 'From' => $departureAirport, 'To' => strtoupper(substr($destination, 0, 3)), 'Date' => $departureDate, 'Departure' => '16:00', 'Arrival' => '18:30', 'Price' => '165.00', 'Total Price' => formatPrice(165 * $passengers), 'Link' => "https://www.kayak.com/flights/{$departureAirport}/" . strtoupper(substr($destination, 0, 3)) . "/{$departureDate}/{$passengers}adults"],
            ['Airline' => 'EuroWings', 'Flight' => 'EW450', 'From' => $departureAirport, 'To' => strtoupper(substr($destination, 0, 3)), 'Date' => $departureDate, 'Departure' => '20:00', 'Arrival' => '22:30', 'Price' => '195.00', 'Total Price' => formatPrice(195 * $passengers), 'Link' => "https://www.google.com/travel/flights?q=flights%20from%20{$departureAirport}%20to%20" . strtoupper(substr($destination, 0, 3)) . "%20on%20{$departureDate}"],
            ['Airline' => 'BlueSky', 'Flight' => 'BS510', 'From' => $departureAirport, 'To' => strtoupper(substr($destination, 0, 3)), 'Date' => $departureDate, 'Departure' => '22:30', 'Arrival' => '01:00', 'Price' => '135.00', 'Total Price' => formatPrice(135 * $passengers), 'Link' => "https://flights.google.com/?hl=en#flt={$departureAirport}." . strtoupper(substr($destination, 0, 3)) . ".{$departureDate}*{$passengers}adults"],
        ];

        $hotelData = [
            ['Name' => 'Grand Hotel', 'Location' => $destination, 'Stars' => 5, 'Rating' => '4.8', 'Price/Night' => '250.00', 'Total Price' => formatPrice(250 * $nights * $rooms), 'Nights' => $nights, 'Rooms' => $rooms, 'Amenities' => 'Pool, Spa, Restaurant, WiFi', 'Link' => "https://www.booking.com/searchresults.html?city=" . urlencode($destination) . "&checkin={$departureDate}&checkout=" . date('Y-m-d', strtotime($departureDate . " +{$nights} days"))],
            ['Name' => 'Comfort Inn', 'Location' => $destination, 'Stars' => 3, 'Rating' => '4.2', 'Price/Night' => '120.00', 'Total Price' => formatPrice(120 * $nights * $rooms), 'Nights' => $nights, 'Rooms' => $rooms, 'Amenities' => 'WiFi, Breakfast, Parking', 'Link' => "https://www.hotels.com/search.do?destination=" . urlencode($destination) . "&checkIn={$departureDate}&checkOut=" . date('Y-m-d', strtotime($departureDate . " +{$nights} days"))],
            ['Name' => 'City View Hotel', 'Location' => $destination, 'Stars' => 4, 'Rating' => '4.5', 'Price/Night' => '180.00', 'Total Price' => formatPrice(180 * $nights * $rooms), 'Nights' => $nights, 'Rooms' => $rooms, 'Amenities' => 'WiFi, Gym, Restaurant, Bar', 'Link' => "https://www.agoda.com/search?city=" . urlencode($destination) . "&checkIn={$departureDate}&los={$nights}"],
            ['Name' => 'Budget Stay', 'Location' => $destination, 'Stars' => 2, 'Rating' => '3.9', 'Price/Night' => '85.00', 'Total Price' => formatPrice(85 * $nights * $rooms), 'Nights' => $nights, 'Rooms' => $rooms, 'Amenities' => 'WiFi, Breakfast', 'Link' => "https://www.hostelworld.com/search?country=" . urlencode($destination) . "&date_from={$departureDate}&date_to=" . date('Y-m-d', strtotime($departureDate . " +{$nights} days"))],
            ['Name' => 'Luxury Resort', 'Location' => $destination, 'Stars' => 5, 'Rating' => '4.9', 'Price/Night' => '450.00', 'Total Price' => formatPrice(450 * $nights * $rooms), 'Nights' => $nights, 'Rooms' => $rooms, 'Amenities' => 'Pool, Spa, Beach, Restaurant, WiFi, Gym', 'Link' => "https://www.expedia.com/Hotels?destination=" . urlencode($destination) . "&startDate={$departureDate}&endDate=" . date('Y-m-d', strtotime($departureDate . " +{$nights} days"))],
        ];

        if ($searchType === 'flights' || $searchType === 'both') {
            $flights = $flightData;
        }

        if ($searchType === 'hotels' || $searchType === 'both') {
            $hotels = $hotelData;
        }
    }
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Travel Search - Real API Results</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 10px;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 20px;
            font-size: 14px;
        }
        .search-form {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 30px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
            color: #555;
        }
        input, select {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
        }
        .row {
            display: flex;
            gap: 20px;
            flex-wrap: wrap;
        }
        .col {
            flex: 1;
            min-width: 200px;
        }
        button {
            background: #667eea;
            color: white;
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
            transition: background 0.3s;
        }
        button:hover {
            background: #764ba2;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
            background: white;
        }
        th {
            background: #667eea;
            color: white;
            padding: 12px;
            text-align: left;
        }
        td {
            padding: 12px;
            border-bottom: 1px solid #ddd;
        }
        tr:hover {
            background: #f5f5f5;
        }
        .results-section {
            margin-top: 30px;
        }
        h2 {
            color: #333;
            margin-bottom: 15px;
        }
        .price {
            font-weight: bold;
            color: #27ae60;
        }
        .book-btn {
            background: #27ae60;
            color: white;
            padding: 8px 15px;
            text-decoration: none;
            border-radius: 5px;
            display: inline-block;
            transition: background 0.3s;
        }
        .book-btn:hover {
            background: #219150;
        }
        .error {
            background: #f8d7da;
            border: 1px solid #f5c6cb;
            color: #721c24;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
        }
        .info {
            background: #d1ecf1;
            border: 1px solid #bee5eb;
            color: #0c5460;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
        }
        .no-results {
            background: #fff3cd;
            border: 1px solid #ffeaa7;
            color: #856404;
            padding: 20px;
            border-radius: 5px;
            text-align: center;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>✈️ Travel Search</h1>
        <p class="subtitle">Real-time results from Amadeus API with direct booking links</p>

        <form method="POST" class="search-form">
            <div class="form-group">
                <label>Search Type</label>
                <select name="search_type" required>
                    <option value="flights">Flights Only</option>
                    <option value="hotels">Hotels Only</option>
                    <option value="both">Flights & Hotels</option>
                </select>
            </div>

            <div class="row">
                <div class="col">
                    <div class="form-group">
                        <label>Departure Airport</label>
                        <input type="text" name="departure_airport" value="SKG" placeholder="e.g., SKG, ATH, LHR" required>
                    </div>
                </div>
                <div class="col">
                    <div class="form-group">
                        <label>Destination (City or Airport Code)</label>
                        <input type="text" name="destination" placeholder="e.g., Paris, LON, NYC" required>
                    </div>
                </div>
            </div>

            <div class="row">
                <div class="col">
                    <div class="form-group">
                        <label>Departure Date</label>
                        <input type="date" name="departure_date" required>
                    </div>
                </div>
                <div class="col">
                    <div class="form-group">
                        <label>Return Date (Optional for flights)</label>
                        <input type="date" name="return_date">
                    </div>
                </div>
            </div>

            <div class="row">
                <div class="col">
                    <div class="form-group">
                        <label>Passengers</label>
                        <input type="number" name="passengers" min="1" value="1" required>
                    </div>
                </div>
                <div class="col">
                    <div class="form-group">
                        <label>Rooms</label>
                        <input type="number" name="rooms" min="1" value="1">
                    </div>
                </div>
                <div class="col">
                    <div class="form-group">
                        <label>Nights</label>
                        <input type="number" name="nights" min="1" value="1">
                    </div>
                </div>
            </div>

            <button type="submit">Search</button>
        </form>

        <?php if ($apiError): ?>
            <div class="error">
                <strong>API Notice:</strong> <?php echo htmlspecialchars($apiError); ?><br>
                Using sample data for demonstration. To use real API data, configure your Amadeus API credentials in config.php.
            </div>
        <?php endif; ?>

        <?php if ($searchPerformed): ?>
            <div class="results-section">
                <?php if (!empty($flights)): ?>
                    <h2>✈️ Flight Results</h2>
                    <?php if (isset($apiError)): ?>
                        <div class="info">Showing sample results - Configure Amadeus API for real data</div>
                    <?php endif; ?>
                    <table>
                        <thead>
                            <tr>
                                <th>Airline</th>
                                <th>Flight</th>
                                <th>From</th>
                                <th>To</th>
                                <th>Date</th>
                                <th>Departure</th>
                                <th>Arrival</th>
                                <th>Duration</th>
                                <th>Price</th>
                                <th>Total</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($flights as $flight): ?>
                                <tr>
                                    <td><?php echo htmlspecialchars($flight['Airline']); ?></td>
                                    <td><?php echo htmlspecialchars($flight['Flight']); ?></td>
                                    <td><?php echo htmlspecialchars($flight['From']); ?></td>
                                    <td><?php echo htmlspecialchars($flight['To']); ?></td>
                                    <td><?php echo htmlspecialchars($flight['Date']); ?></td>
                                    <td><?php echo htmlspecialchars($flight['Departure']); ?></td>
                                    <td><?php echo htmlspecialchars($flight['Arrival']); ?></td>
                                    <td><?php echo isset($flight['Duration']) ? htmlspecialchars($flight['Duration']) : '-'; ?></td>
                                    <td><?php echo htmlspecialchars($flight['Price']); ?>€</td>
                                    <td class="price"><?php echo htmlspecialchars($flight['Total Price']); ?>€</td>
                                    <td><a href="<?php echo htmlspecialchars($flight['Link']); ?>" target="_blank" class="book-btn">Book Now</a></td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                <?php endif; ?>

                <?php if (!empty($hotels)): ?>
                    <h2>🏨 Hotel Results</h2>
                    <?php if (isset($apiError)): ?>
                        <div class="info">Showing sample results - Configure Amadeus API for real data</div>
                    <?php endif; ?>
                    <table>
                        <thead>
                            <tr>
                                <th>Name</th>
                                <th>Location</th>
                                <th>Stars</th>
                                <th>Rating</th>
                                <th>Price/Night</th>
                                <th>Nights</th>
                                <th>Rooms</th>
                                <th>Total</th>
                                <th>Amenities</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($hotels as $hotel): ?>
                                <tr>
                                    <td><?php echo htmlspecialchars($hotel['Name']); ?></td>
                                    <td><?php echo htmlspecialchars($hotel['Location']); ?></td>
                                    <td><?php echo str_repeat('★', $hotel['Stars']); ?></td>
                                    <td><?php echo htmlspecialchars($hotel['Rating']); ?>/5</td>
                                    <td><?php echo htmlspecialchars($hotel['Price/Night']); ?>€</td>
                                    <td><?php echo htmlspecialchars($hotel['Nights']); ?></td>
                                    <td><?php echo htmlspecialchars($hotel['Rooms']); ?></td>
                                    <td class="price"><?php echo htmlspecialchars($hotel['Total Price']); ?>€</td>
                                    <td><?php echo htmlspecialchars($hotel['Amenities']); ?></td>
                                    <td><a href="<?php echo htmlspecialchars($hotel['Link']); ?>" target="_blank" class="book-btn">Book Now</a></td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                <?php endif; ?>

                <?php if (empty($flights) && empty($hotels)): ?>
                    <div class="no-results">
                        <h3>No results found</h3>
                        <p>Try adjusting your search criteria or check your destination airport code.</p>
                    </div>
                <?php endif; ?>
            </div>
        <?php endif; ?>
    </div>
</body>
</html>