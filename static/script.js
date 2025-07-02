// static/script.js - Refined with better loading states and transitions + HISTORY DISPLAY

document.addEventListener('DOMContentLoaded', function() {
    const cameraImage = document.getElementById('camera-image');
    const predictionResults = document.getElementById('prediction-results');
    const loadingOverlay = document.getElementById('loading-overlay');
    const loadingStatus = document.getElementById('loading-status');
    const historyContainer = document.getElementById('history-container'); // NEW

    let lastTimestamp = 0; // To track the last received data timestamp
    let lastHistoryUpdateTimestamp = 0; // NEW: To track last history update

    // Function to fetch and update live feed and prediction
    function updateLiveFeed() {
        fetch('/latest_data')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json(); // Parse response as JSON
            })
            .then(data => {
                // Check if a valid image URL exists and if the data is newer
                if (data.image_url && data.timestamp > lastTimestamp) {
                    console.log('New live data received. Updating.');

                    // Update camera image
                    cameraImage.src = data.image_url;
                    cameraImage.onload = () => {
                        cameraImage.style.opacity = 1;
                        if (!loadingOverlay.classList.contains('hidden')) {
                            loadingOverlay.classList.add('hidden');
                        }
                    };
                    cameraImage.onerror = () => {
                        console.error('Failed to load image:', data.image_url);
                        loadingStatus.textContent = 'Error loading image. Retrying...';
                        loadingOverlay.classList.remove('hidden');
                        cameraImage.style.opacity = 0;
                    };

                    // Update prediction results
                    if (data.prediction) {
                        const predictions = data.prediction.split('<br>').filter(line => line.trim() !== ''); // Use <br> for splitting
                        let predictionHtml = '<ul>';
                        // Start from index 1 as Flask sends "Prediksi:<br>1. class (score)"
                        predictions.slice(1).forEach(p => { 
                            const match = p.match(/^\d+\.\s*(.+?)\s*\(([\d.]+)\)$/);
                            if (match) {
                                const label = match[1].trim();
                                const score = parseFloat(match[2]).toFixed(4);
                                predictionHtml += `<li><span class="prediction-label">${label}</span><span class="prediction-score">${score}</span></li>`;
                            } else {
                                predictionHtml += `<li>${p}</li>`;
                            }
                        });
                        predictionHtml += '</ul>';
                        predictionResults.innerHTML = predictionHtml;
                    } else {
                        predictionResults.innerHTML = '<p class="initial-message">No prediction available.</p>';
                    }

                    lastTimestamp = data.timestamp; // Update last timestamp

                } else if (!data.image_url) {
                    loadingStatus.textContent = 'Waiting for the first image...';
                    loadingOverlay.classList.remove('hidden');
                    cameraImage.style.opacity = 0;
                    predictionResults.innerHTML = '<p class="initial-message">Awaiting first image for analysis...</p>';
                }
            })
            .catch(error => {
                console.error('Error fetching latest data:', error);
                predictionResults.innerHTML = '<p class="initial-message">Error fetching data. Please check server connection.</p>';
                loadingStatus.textContent = 'Connection error. Retrying...';
                loadingOverlay.classList.remove('hidden');
                cameraImage.style.opacity = 0;
            });
    }


    // NEW: Function to fetch and update history data
    function updateHistory() {
        fetch('/history_data')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json(); // Array of history items
            })
            .then(historyData => {
                // Only update if history data has changed (e.g., new item added)
                // A simple check: compare the timestamp of the first (latest) item
                if (historyData.length > 0 && historyData[0].timestamp_formatted !== lastHistoryUpdateTimestamp) {
                    console.log('New history data received. Updating history display.');
                    historyContainer.innerHTML = ''; // Clear previous history
                    
                    historyData.forEach(item => {
                        const historyItemDiv = document.createElement('div');
                        historyItemDiv.classList.add('history-item');

                        const imageWrapper = document.createElement('div');
                        imageWrapper.classList.add('history-item-image-wrapper');
                        const img = document.createElement('img');
                        img.src = item.image_url;
                        img.alt = 'History Image';
                        img.classList.add('history-item-image');
                        imageWrapper.appendChild(img);

                        const detailsDiv = document.createElement('div');
                        detailsDiv.classList.add('history-item-details');

                        const timestampP = document.createElement('p');
                        timestampP.classList.add('history-timestamp');
                        timestampP.textContent = item.timestamp_formatted;
                        detailsDiv.appendChild(timestampP);

                        // Display top prediction only in history for brevity
                        if (item.predictions && item.predictions.length > 0) {
                            const topPrediction = item.predictions[0]; // Assuming first is best
                            const predictionLabelP = document.createElement('p');
                            predictionLabelP.classList.add('history-prediction-label');
                            
                            const labelSpan = document.createElement('span');
                            labelSpan.textContent = topPrediction.label;
                            predictionLabelP.appendChild(labelSpan);

                            const scoreSpan = document.createElement('span');
                            scoreSpan.classList.add('history-prediction-score');
                            scoreSpan.textContent = topPrediction.score.toFixed(4); // Format score
                            predictionLabelP.appendChild(scoreSpan);
                            
                            detailsDiv.appendChild(predictionLabelP);
                        } else {
                            const noPredP = document.createElement('p');
                            noPredP.textContent = "No prediction";
                            noPredP.classList.add('history-prediction-label');
                            detailsDiv.appendChild(noPredP);
                        }

                        historyItemDiv.appendChild(imageWrapper);
                        historyItemDiv.appendChild(detailsDiv);
                        historyContainer.appendChild(historyItemDiv);
                    });

                    // Update last history update timestamp
                    lastHistoryUpdateTimestamp = historyData[0].timestamp_formatted;
                } else if (historyData.length === 0) {
                    historyContainer.innerHTML = '<p class="initial-message">No history yet. Images will appear here after processing...</p>';
                    lastHistoryUpdateTimestamp = 0; // Reset if history becomes empty
                }
            })
            .catch(error => {
                console.error('Error fetching history data:', error);
                historyContainer.innerHTML = '<p class="initial-message">Error loading history.</p>';
            });
    }


    // Initial setup: show loading overlay for camera and default prediction message
    loadingOverlay.classList.remove('hidden');
    loadingStatus.textContent = 'Establishing connection...';
    predictionResults.innerHTML = '<p class="initial-message">Awaiting first image for analysis...</p>';
    historyContainer.innerHTML = '<p class="initial-message">Loading history...</p>'; // Initial history loading message

    // Call updates immediately on page load
    updateLiveFeed();
    updateHistory();


    // Set intervals for periodic updates
    const liveFeedInterval = 2500; // 2.5 seconds
    const historyInterval = 5000; // 5 seconds (can be less frequent than live feed)

    setInterval(updateLiveFeed, liveFeedInterval);
    setInterval(updateHistory, historyInterval);
});