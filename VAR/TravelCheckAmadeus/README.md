# Travel Search with Real API Integration

This application uses the **Amadeus API** to provide real-time flight and hotel search results with direct booking links.

## Setup Instructions

### 1. Get Amadeus API Credentials

1. Go to [developers.amadeus.com](https://developers.amadeus.com)
2. Create a free account
3. Navigate to "My Apps" and create a new self-service application
4. Copy your **API Key** (Client ID) and **API Secret** (Client Secret)

### 2. Configure the Application

Edit `config.php` and replace the placeholder values:

```php
'amadeus' => [
    'client_id' => 'YOUR_AMADEUS_CLIENT_ID',  // Paste your API Key here
    'client_secret' => 'YOUR_AMADEUS_CLIENT_SECRET',  // Paste your API Secret here
    ...
]
```

Or set environment variables:

```bash
export AMADEUS_CLIENT_ID="your_api_key"
export AMADEUS_CLIENT_SECRET="your_api_secret"
```

### 3. Test the Application

Access the application at: `http://localhost/CompareDomains/index.php`

## Features

- ✈️ **Real-time flight searches** from Amadeus API
- 🏨 **Real-time hotel searches** with actual availability and pricing
- 🔗 **Direct booking links** that take users to the exact flight/hotel
- 🌍 **City code resolution** - type city names or airport codes
- 💰 **Price calculation** - automatically calculates total based on passengers/rooms/nights
- 📅 **Flexible dates** - one-way or round-trip flights
- 🎨 **Responsive design** - works on all devices

## How It Works

### Flight Search
1. User enters departure airport (default: SKG) and destination
2. Application resolves city/airport codes using Amadeus Location API
3. Calls Amadeus Flight Offers API with search parameters
4. Returns real flight options with:
   - Airline and flight number
   - Departure/arrival times and duration
   - Real pricing in EUR
   - Direct booking links

### Hotel Search
1. Application searches for hotels in the destination city
2. Returns available hotels with:
   - Hotel name and star rating
   - Real room rates per night
   - Available amenities
   - Direct Booking.com links with pre-filled search

## API Endpoints Used

- `POST /v1/security/oauth2/token` - Authentication
- `GET /v1/reference-data/locations` - City/airport code lookup
- `GET /v2/shopping/flight-offers` - Flight search
- `POST /v3/shopping/hotel-offers` - Hotel search

## Fallback Mode

If API credentials are not configured or API is unavailable, the application falls back to sample data with demo booking links to:
- Skyscanner
- Expedia
- Booking.com
- Kayak
- Google Flights

## File Structure

```
CompareDomains/
├── index.php          # Main application
├── AmadeusAPI.php     # API client class
├── config.php         # API configuration
└── README.md          # This file
```

## Common Issues

**"Authentication failed"** - Check your API credentials in config.php

**"City not found"** - Try using IATA airport codes (e.g., LON, NYC, PAR) instead of city names

**No results found** - Check dates are in the future and airport codes are valid

## API Limits

Amadeus free tier includes:
- 2,000 API calls/month
- Rate limited to 10 requests/second

For production use, consider upgrading to a paid tier.

## Support

- Amadeus API Documentation: [developers.amadeus.com/docs](https://developers.amadeus.com/docs)
- API Support: support@amadeus.com