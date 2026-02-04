/* ============================================================
   ADAPTIVE LIBRARY SEARCH - PREMIUM ANIMATIONS & INTERACTIONS
   Cinematic Micro-interactions, Scroll Effects & Parallax
   ============================================================ */

// ============================================================
// PAGE LOAD & INITIALIZATION
// ============================================================

document.addEventListener('DOMContentLoaded', () => {
    initializeAnimations();
    initializeScrollEffects();
    initializeMicroInteractions();
    initializeSearchAnimations();
    initializeFormAnimations();
});

// ============================================================
// INITIALIZATION FUNCTIONS
// ============================================================

function initializeAnimations() {
    // Animate hero title with stagger effect
    // Animate hero title with stagger effect
    const heroTitle = document.querySelector('.hero-title-animated'); // Changed class to avoid conflict
    if (heroTitle) {
        animateTitleReveal(heroTitle);
    }

    // Animate featured book cards
    const bookCards = document.querySelectorAll('.book-card');
    bookCards.forEach((card, index) => {
        card.style.animationDelay = `${index * 0.1}s`;
    });

    // Animate feature cards
    const featureCards = document.querySelectorAll('.feature-card');
    featureCards.forEach((card, index) => {
        card.style.animationDelay = `${index * 0.1}s`;
    });

    // Animate stat cards
    const statCards = document.querySelectorAll('.stat-card');
    statCards.forEach((card, index) => {
        card.style.animationDelay = `${index * 0.15}s`;
    });
}

function initializeScrollEffects() {
    const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -100px 0px'
    };

    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('animate-in');
                observer.unobserve(entry.target);
            }
        });
    }, observerOptions);

    // Observe all elements with scroll animation classes
    document.querySelectorAll('[data-scroll-animate]').forEach(el => {
        observer.observe(el);
    });

    // Parallax effect for hero section
    window.addEventListener('scroll', handleParallax);
}

function initializeMicroInteractions() {
    // Button hover effects
    document.querySelectorAll('button, a[class*="btn"]').forEach(btn => {
        btn.addEventListener('mouseenter', (e) => {
            createRipple(e);
        });
    });

    // Magnetic navigation links
    document.querySelectorAll('.nav-link').forEach(link => {
        link.addEventListener('mousemove', (e) => {
            const rect = link.getBoundingClientRect();
            const x = e.clientX - rect.left - rect.width / 2;
            const y = e.clientY - rect.top - rect.height / 2;

            const moveX = x * 0.3;
            const moveY = y * 0.3;

            link.style.transform = `translate(${moveX}px, ${moveY}px)`;
        });

        link.addEventListener('mouseleave', () => {
            link.style.transform = 'translate(0, 0)';
        });
    });

    // Card hover depth effect
    document.querySelectorAll('.book-card, .feature-card, .stat-card, .profile-card, .borrowed-book-item').forEach(card => {
        card.addEventListener('mousemove', (e) => {
            const rect = card.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;

            const rotateX = ((y - rect.height / 2) / rect.height) * 5;
            const rotateY = ((x - rect.width / 2) / rect.width) * 5;

            card.style.transform = `perspective(1000px) rotateX(${rotateX}deg) rotateY(${rotateY}deg)`;
        });

        card.addEventListener('mouseleave', () => {
            card.style.transform = 'perspective(1000px) rotateX(0) rotateY(0)';
        });
    });

    // Smooth transitions
    document.querySelectorAll('a').forEach(link => {
        link.addEventListener('click', (e) => {
            if (link.href && !link.target && link.origin === window.location.origin) {
                e.preventDefault();
                fadeOutAndNavigate(link.href);
            }
        });
    });
}

function initializeSearchAnimations() {
    const searchInput = document.querySelector('.search-input');
    if (searchInput) {
        let debounceTimer;

        searchInput.addEventListener('input', (e) => {
            clearTimeout(debounceTimer);
            debounceTimer = setTimeout(() => {
                animateResultsEntry();
            }, 300);
        });

        searchInput.addEventListener('focus', () => {
            searchInput.closest('.search-input-wrapper')?.classList.add('focused');
        });

        searchInput.addEventListener('blur', () => {
            searchInput.closest('.search-input-wrapper')?.classList.remove('focused');
        });
    }

    // Filter button animations
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.addEventListener('click', (e) => {
            e.preventDefault();
            animateFilterChange(btn);
        });
    });

    // View toggle animations
    document.querySelectorAll('.view-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            animateViewChange(btn);
        });
    });
}

function initializeFormAnimations() {
    // Form input focus effects
    document.querySelectorAll('.form-input').forEach(input => {
        input.addEventListener('focus', () => {
            input.closest('.form-group')?.classList.add('focused');
        });

        input.addEventListener('blur', () => {
            input.closest('.form-group')?.classList.remove('focused');
        });
    });

    // Radio button animations
    document.querySelectorAll('input[type="radio"]').forEach(radio => {
        radio.addEventListener('change', (e) => {
            animateRadioChange(e.target);
        });
    });

    // Form submission
    document.querySelectorAll('form').forEach(form => {
        form.addEventListener('submit', (e) => {
            animateFormSubmit(e);
        });
    });
}

// ============================================================
// ANIMATION EFFECTS
// ============================================================

function animateTitleReveal(element) {
    const text = element.textContent;
    element.innerHTML = '';

    const spans = text.split('').map(char => {
        const span = document.createElement('span');
        span.textContent = char;
        span.style.opacity = '0';
        span.style.display = 'inline-block';
        span.style.animation = `fadeInChar 0.6s ease-out forwards`;
        return span;
    });

    spans.forEach((span, index) => {
        span.style.animationDelay = `${index * 0.05}s`;
        element.appendChild(span);
    });

    // Add CSS animation
    const style = document.createElement('style');
    style.textContent = `
        @keyframes fadeInChar {
            from {
                opacity: 0;
                transform: translateY(20px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
    `;
    document.head.appendChild(style);
}

function handleParallax() {
    const scrollY = window.scrollY;
    const heroGradient = document.querySelector('.hero-gradient');
    const gradientBg = document.querySelector('.gradient-bg');

    if (heroGradient) {
        heroGradient.style.transform = `translateY(${scrollY * 0.5}px)`;
    }

    if (gradientBg) {
        gradientBg.style.transform = `translateY(${scrollY * 0.3}px)`;
    }
}

function createRipple(e) {
    const button = e.target.closest('button, a[class*="btn"]');
    if (!button) return;

    const ripple = document.createElement('span');
    const rect = button.getBoundingClientRect();
    const size = Math.max(rect.width, rect.height);
    const x = e.clientX - rect.left - size / 2;
    const y = e.clientY - rect.top - size / 2;

    ripple.style.cssText = `
        position: absolute;
        width: ${size}px;
        height: ${size}px;
        background: rgba(255, 255, 255, 0.5);
        border-radius: 50%;
        left: ${x}px;
        top: ${y}px;
        pointer-events: none;
        animation: ripple 0.6s ease-out;
    `;

    const style = document.createElement('style');
    if (!document.querySelector('style[data-ripple]')) {
        style.setAttribute('data-ripple', 'true');
        style.textContent = `
            @keyframes ripple {
                from {
                    opacity: 1;
                    transform: scale(0);
                }
                to {
                    opacity: 0;
                    transform: scale(1);
                }
            }
        `;
        document.head.appendChild(style);
    }

    if (!button.style.position || button.style.position === 'static') {
        button.style.position = 'relative';
    }
    button.appendChild(ripple);

    setTimeout(() => ripple.remove(), 600);
}

function animateResultsEntry() {
    const resultCards = document.querySelectorAll('.result-card, .book-card');
    resultCards.forEach((card, index) => {
        card.style.opacity = '0';
        card.style.transform = 'translateY(30px)';
        setTimeout(() => {
            card.style.transition = 'opacity 0.5s ease-out, transform 0.5s ease-out';
            card.style.opacity = '1';
            card.style.transform = 'translateY(0)';
        }, index * 50);
    });
}

function animateFilterChange(button) {
    button.style.transform = 'scale(0.95)';
    setTimeout(() => {
        button.style.transform = 'scale(1)';
    }, 150);
}

function animateViewChange(button) {
    const container = document.querySelector('.results-container');
    if (container) {
        container.style.opacity = '0';
        setTimeout(() => {
            container.style.opacity = '1';
        }, 200);
    }
}

function animateRadioChange(radio) {
    const label = radio.closest('.type-label');
    if (label) {
        label.style.transform = 'scale(0.95)';
        setTimeout(() => {
            label.style.transform = 'scale(1)';
        }, 150);
    }
}

function animateFormSubmit(e) {
    const form = e.target;
    const button = form.querySelector('button[type="submit"]');

    if (button) {
        button.classList.add('btn-loading');
        button.disabled = true;
    }
}

function fadeOutAndNavigate(url) {
    document.body.style.opacity = '0';
    document.body.style.transition = 'opacity 0.3s ease-out';

    setTimeout(() => {
        window.location.href = url;
    }, 300);
}

// ============================================================
// SCROLL REVEAL ANIMATIONS
// ============================================================

function addScrollRevealCSS() {
    const style = document.createElement('style');
    style.textContent = `
        [data-scroll-animate] {
            opacity: 0;
            transform: translateY(30px);
            transition: opacity 0.6s ease-out, transform 0.6s ease-out;
        }

        [data-scroll-animate].animate-in {
            opacity: 1;
            transform: translateY(0);
        }

        [data-scroll-animate="fade"] {
            opacity: 0;
        }

        [data-scroll-animate="fade"].animate-in {
            opacity: 1;
        }

        [data-scroll-animate="slide-left"] {
            transform: translateX(-30px);
        }

        [data-scroll-animate="slide-left"].animate-in {
            transform: translateX(0);
        }

        [data-scroll-animate="slide-right"] {
            transform: translateX(30px);
        }

        [data-scroll-animate="slide-right"].animate-in {
            transform: translateX(0);
        }

        /* Smooth transitions for state changes */
        * {
            transition: background-color 0.3s ease, color 0.3s ease, border-color 0.3s ease;
        }

        button, a {
            transition: all 0.3s ease;
        }
    `;
    document.head.appendChild(style);
}

addScrollRevealCSS();

// ============================================================
// UTILITY: SHOW NOTIFICATION TOAST
// ============================================================

function showNotification(message, type = 'success') {
    const existingToast = document.querySelector('.notification-toast');
    if (existingToast) {
        existingToast.remove();
    }

    const toast = document.createElement('div');
    toast.className = `notification-toast ${type}`;
    toast.textContent = message;
    document.body.appendChild(toast);

    requestAnimationFrame(() => {
        toast.classList.add('show');
    });

    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => toast.remove(), 400);
    }, 3000);
}

// ============================================================
// NAVBAR ANIMATIONS
// ============================================================

window.addEventListener('scroll', () => {
    const navbar = document.querySelector('.navbar');
    if (navbar) {
        if (window.scrollY > 20) {
            navbar.style.boxShadow = '0 4px 20px rgba(0, 0, 0, 0.3)';
        } else {
            navbar.style.boxShadow = '0 4px 6px rgba(0, 0, 0, 0.2)';
        }
    }
});

// ============================================================
// BUTTON STATE ANIMATIONS
// ============================================================

document.addEventListener('click', (e) => {
    const button = e.target.closest('button');
    if (button && !button.classList.contains('btn-loading')) {
        button.classList.add('btn-active');
        setTimeout(() => {
            button.classList.remove('btn-active');
        }, 200);
    }
});

// ============================================================
// FOCUS MANAGEMENT
// ============================================================

const focusableElements = 'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])';
const modal = document.querySelector('.modal');

document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape' && modal?.classList.contains('show')) {
        modal.classList.remove('show');
    }
});

// ============================================================
// CUSTOM SCROLL BEHAVIOR
// ============================================================

document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
        e.preventDefault();
        const target = document.querySelector(this.getAttribute('href'));
        if (target) {
            target.scrollIntoView({
                behavior: 'smooth',
                block: 'start'
            });
        }
    });
});

// ============================================================
// ANIMATION OPTIMIZATION
// ============================================================

// Reduce animations for users who prefer reduced motion
if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
    document.documentElement.style.setProperty('--transition', 'all 0.01s linear');
    document.querySelectorAll('*').forEach(el => {
        el.style.animation = 'none !important';
    });
}

// ============================================================
// PERFORMANCE: LAZY LOAD IMAGES
// ============================================================

if ('IntersectionObserver' in window) {
    const imageObserver = new IntersectionObserver((entries, observer) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const img = entry.target;
                if (img.dataset.src) {
                    img.src = img.dataset.src;
                    img.classList.add('loaded');
                }
                observer.unobserve(img);
            }
        });
    });

    document.querySelectorAll('img[data-src]').forEach(img => {
        imageObserver.observe(img);
    });
}

// ============================================================
// PAGE VISIBILITY DETECTION
// ============================================================

document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // Pause animations when page is hidden
        document.documentElement.style.animationPlayState = 'paused';
    } else {
        // Resume animations when page is visible
        document.documentElement.style.animationPlayState = 'running';
    }
});

// ============================================================
// RESPONSIVE ANIMATION ADJUSTMENTS
// ============================================================

const mediaQuery = window.matchMedia('(max-width: 768px)');

function handleMediaChange(e) {
    if (e.matches) {
        // Reduce animation complexity on mobile
        document.querySelectorAll('.book-card, .feature-card').forEach(el => {
            el.style.animationDuration = '0.3s';
        });
    } else {
        // Full animations on desktop
        document.querySelectorAll('.book-card, .feature-card').forEach(el => {
            el.style.animationDuration = '';
        });
    }
}

mediaQuery.addEventListener('change', handleMediaChange);
handleMediaChange(mediaQuery);

console.log('Premium animations initialized ✨');
