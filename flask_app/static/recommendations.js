/* Shared recommendations logic - tracks recent books and fetches personalized recommendations */

const RECENT_BOOKS_KEY = 'library_recent_books';
const MAX_RECENT = 10;

function addToRecentBooks(isbns) {
    if (!Array.isArray(isbns) || isbns.length === 0) return;
    try {
        let recent = JSON.parse(sessionStorage.getItem(RECENT_BOOKS_KEY) || '[]');
        for (const isbn of isbns) {
            recent = recent.filter(id => id !== isbn);
            recent.unshift(isbn);
        }
        recent = recent.slice(0, MAX_RECENT);
        sessionStorage.setItem(RECENT_BOOKS_KEY, JSON.stringify(recent));
    } catch (e) {}
}

function getRecentBooks() {
    try {
        return JSON.parse(sessionStorage.getItem(RECENT_BOOKS_KEY) || '[]');
    } catch (e) {
        return [];
    }
}

function getCategoryColor(category) {
    const colors = {
        'Programming': 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)',
        'Software Engineering': 'linear-gradient(135deg, #f093fb 0%, #f5576c 100%)',
        'Machine Learning': 'linear-gradient(135deg, #4facfe 0%, #00f2fe 100%)',
        'Data Science': 'linear-gradient(135deg, #fa709a 0%, #fee140 100%)',
        'Algorithms': 'linear-gradient(135deg, #43e97b 0%, #38f9d7 100%)',
        'Web Development': 'linear-gradient(135deg, #a8edea 0%, #fed6e3 100%)',
        'Networking': 'linear-gradient(135deg, #ff9a56 0%, #ff6a88 100%)',
        'Cloud Computing': 'linear-gradient(135deg, #5f72bd 0%, #9921e8 100%)',
    };
    return colors[category] || 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)';
}

function escapeHtml(unsafe) {
    if (!unsafe) return '';
    return String(unsafe)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}

async function loadRecommendations(containerId, options = {}) {
    const container = document.getElementById(containerId);
    if (!container) return;

    container.innerHTML = `
        <div class="recommendations-loading">
            <div class="loading-spinner" style="margin: 20px auto;"></div>
            <p>Loading recommendations...</p>
        </div>
    `;

    try {
        const recentISBNs = getRecentBooks();
        const response = await fetch('/api/recommendations/personalized', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ recentISBNs })
        });
        const data = await response.json();

        if (data.success && data.results && data.results.length > 0) {
            container.innerHTML = data.results.map((book, index) => {
                const categoryColor = getCategoryColor(book.category);
                const availability = book.availableCopies > 0 ? 'available' : 'unavailable';
                return `
                    <div class="book-card recommendation-card" style="animation-delay: ${index * 50}ms">
                        <div class="book-cover" style="background: ${categoryColor};">
                            <div class="cover-overlay">${(book.category || '').substring(0, 3).toUpperCase()}</div>
                        </div>
                        <div class="book-info">
                            <h3 class="book-title">${escapeHtml(book.title)}</h3>
                            <p class="book-author">${escapeHtml(book.author)}</p>
                            <div class="book-meta">
                                <span class="category">${escapeHtml(book.category)}</span>
                                <span class="availability ${availability}">${book.availableCopies}/${book.totalCopies} available</span>
                            </div>
                            <div class="book-actions">
                                ${book.availableCopies > 0 ?
                                    `<button class="btn-small btn-outline" onclick="issueBook('${book.isbn}'); if(typeof refreshRecommendations==='function') refreshRecommendations();">Issue</button>` :
                                    `<button class="btn-small btn-outline disabled">Out of Stock</button>`
                                }
                                <button class="btn-small btn-ghost" onclick="reserveBook('${book.isbn}'); if(typeof refreshRecommendations==='function') refreshRecommendations();">Reserve</button>
                            </div>
                        </div>
                    </div>
                `;
            }).join('');
        } else {
            container.innerHTML = `
                <div class="recommendations-empty">
                    <div class="empty-icon">⭐</div>
                    <p>No recommendations yet. Borrow some books or search for titles to get personalized suggestions!</p>
                </div>
            `;
        }
    } catch (error) {
        container.innerHTML = `
            <div class="recommendations-empty">
                <div class="empty-icon">⚠️</div>
                <p>Could not load recommendations. Try again later.</p>
            </div>
        `;
    }
}
