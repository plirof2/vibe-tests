<?php
class AmadeusAPI {
    private $config;
    private $accessToken;
    private $tokenExpiresAt;

    public function __construct($config) {
        $this->config = $config;
    }

    private function authenticate() {
        if ($this->accessToken && $this->tokenExpiresAt > time()) {
            return $this->accessToken;
        }

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $this->config['amadeus']['auth_url']);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query([
            'grant_type' => 'client_credentials',
            'client_id' => $this->config['amadeus']['client_id'],
            'client_secret' => $this->config['amadeus']['client_secret']
        ]));
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Content-Type: application/x-www-form-urlencoded'
        ]);

        $response = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        if ($httpCode !== 200) {
            throw new Exception('Authentication failed: ' . $response);
        }

        $data = json_decode($response, true);
        $this->accessToken = $data['access_token'];
        $this->tokenExpiresAt = time() + $data['expires_in'] - 60;

        return $this->accessToken;
    }

    private function request($endpoint, $params = [], $method = 'GET') {
        $token = $this->authenticate();
        $url = $this->config['amadeus']['base_url'] . $endpoint;

        if ($method === 'GET' && !empty($params)) {
            $url .= '?' . http_build_query($params);
        }

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Authorization: Bearer ' . $token
        ]);

        if ($method === 'POST') {
            curl_setopt($ch, CURLOPT_POST, true);
            curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($params));
            curl_setopt($ch, CURLOPT_HTTPHEADER, [
                'Authorization: Bearer ' . $token,
                'Content-Type: application/json'
            ]);
        }

        $response = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        if ($httpCode < 200 || $httpCode >= 300) {
            throw new Exception("API request failed (HTTP $httpCode): $response");
        }

        return json_decode($response, true);
    }

    public function searchFlights($origin, $destination, $departureDate, $passengers = 1, $returnDate = null) {
        $params = [
            'originLocationCode' => strtoupper($origin),
            'destinationLocationCode' => strtoupper($destination),
            'departureDate' => $departureDate,
            'adults' => $passengers,
            'max' => 20,
            'currencyCode' => 'EUR'
        ];

        if ($returnDate) {
            $params['returnDate'] = $returnDate;
        }

        return $this->request($this->config['amadeus']['flights_url'], $params);
    }

    public function getCityCode($cityName) {
        $params = [
            'keyword' => $cityName,
            'subType' => 'CITY,AIRPORT',
            'max' => 1
        ];

        $response = $this->request('/v1/reference-data/locations', $params);

        if (!empty($response['data'])) {
            return $response['data'][0]['iataCode'];
        }

        throw new Exception("City not found: $cityName");
    }

    public function searchHotels($cityCode, $checkInDate, $rooms = 1, $nights = 1) {
        $checkOutDate = date('Y-m-d', strtotime($checkInDate . " +$nights days"));

        $params = [
            'hotelIds' => 'ALL',
            'cityCode' => strtoupper($cityCode),
            'checkInDate' => $checkInDate,
            'checkOutDate' => $checkOutDate,
            'roomQuantity' => $rooms,
            'adults' => $rooms,
            'currency' => 'EUR',
            'bestRateOnly' => 'true'
        ];

        return $this->request($this->config['amadeus']['hotel_offers_url'], $params, 'POST');
    }

    public function getCityCodeFromName($cityName) {
        try {
            return $this->getCityCode($cityName);
        } catch (Exception $e) {
            return strtoupper(substr($cityName, 0, 3));
        }
    }
}