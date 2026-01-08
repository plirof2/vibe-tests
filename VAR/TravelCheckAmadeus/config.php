<?php
return [
    'amadeus' => [
        'client_id' => getenv('AMADEUS_CLIENT_ID') ?: 'YOUR_AMADEUS_CLIENT_ID',
        'client_secret' => getenv('AMADEUS_CLIENT_SECRET') ?: 'YOUR_AMADEUS_CLIENT_SECRET',
        'base_url' => 'https://test.api.amadeus.com',
        'auth_url' => 'https://test.api.amadeus.com/v1/security/oauth2/token',
        'flights_url' => '/v2/shopping/flight-offers',
        'hotels_url' => '/v1/reference-data/locations/hotels/by-city',
        'hotel_offers_url' => '/v3/shopping/hotel-offers',
    ]
];