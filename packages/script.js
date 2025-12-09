// ReproVM Wiki - Interactive Features

document.addEventListener('DOMContentLoaded', () => {
    initSmoothScroll();
    initMobileMenu();
    initCodeCopy();
    initScrollAnimations();
    initMermaidDiagrams();
    initThemeToggle();
    initSearchFunctionality();
    initProgressBar();
    initActiveNavLinks();
});

// Smooth scrolling for navigation links
function initSmoothScroll() {
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
}

// Mobile menu toggle
function initMobileMenu() {
    const menuToggle = document.querySelector('.menu-toggle');
    const navLinks = document.querySelector('.nav-links');
    const navLinkItems = document.querySelectorAll('.nav-links a');
    
    if (menuToggle && navLinks) {
        menuToggle.addEventListener('click', () => {
            menuToggle.classList.toggle('active');
            navLinks.classList.toggle('active');
        });
        
        // Close menu when clicking a link
        navLinkItems.forEach(link => {
            link.addEventListener('click', () => {
                menuToggle.classList.remove('active');
                navLinks.classList.remove('active');
            });
        });
        
        // Close menu when clicking outside
        document.addEventListener('click', (e) => {
            if (!menuToggle.contains(e.target) && !navLinks.contains(e.target)) {
                menuToggle.classList.remove('active');
                navLinks.classList.remove('active');
            }
        });
    }
}

// Copy code to clipboard
function initCodeCopy() {
    document.querySelectorAll('.code-block').forEach(block => {
        const pre = block.querySelector('pre');
        
        // Wrap pre in a scrollable div
        const wrapper = document.createElement('div');
        wrapper.className = 'code-wrapper';
        pre.parentNode.insertBefore(wrapper, pre);
        wrapper.appendChild(pre);
        
        // Create copy button
        const copyBtn = document.createElement('button');
        copyBtn.className = 'copy-btn';
        copyBtn.innerHTML = '📋';
        
        block.appendChild(copyBtn);
        
        copyBtn.addEventListener('click', async () => {
            const code = pre.textContent;
            try {
                await navigator.clipboard.writeText(code);
                copyBtn.innerHTML = '✓';
                copyBtn.style.background = 'var(--success-color)';
                setTimeout(() => {
                    copyBtn.innerHTML = '📋';
                    copyBtn.style.background = 'var(--primary-color)';
                }, 2000);
            } catch (err) {
                copyBtn.innerHTML = '✗';
                copyBtn.style.background = 'var(--danger-color)';
                setTimeout(() => {
                    copyBtn.innerHTML = '📋';
                    copyBtn.style.background = 'var(--primary-color)';
                }, 2000);
            }
        });
    });
}

// Scroll animations
function initScrollAnimations() {
    const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -100px 0px'
    };
    
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('fade-in-up');
                observer.unobserve(entry.target);
            }
        });
    }, observerOptions);
    
    document.querySelectorAll('.feature-card, .stat-card, .timeline-item').forEach(el => {
        observer.observe(el);
    });
}

// Initialize Mermaid diagrams
function initMermaidDiagrams() {
    if (typeof mermaid !== 'undefined') {
        mermaid.initialize({
            startOnLoad: true,
            theme: 'dark',
            themeVariables: {
                primaryColor: '#2563eb',
                primaryTextColor: '#f8fafc',
                primaryBorderColor: '#334155',
                lineColor: '#64748b',
                secondaryColor: '#7c3aed',
                tertiaryColor: '#10b981',
                background: '#1e293b',
                mainBkg: '#1e293b',
                secondBkg: '#0f172a',
                borderColor: '#334155',
                fontSize: '16px',
                fontFamily: 'Inter, system-ui, sans-serif'
            },
            flowchart: {
                curve: 'basis',
                padding: 20
            }
        });
    }
}

// Theme toggle (light/dark)
function initThemeToggle() {
    const toggleBtn = document.querySelector('.theme-toggle');
    if (!toggleBtn) return;
    
    const currentTheme = localStorage.getItem('theme') || 'dark';
    document.documentElement.setAttribute('data-theme', currentTheme);
    
    toggleBtn.addEventListener('click', () => {
        const theme = document.documentElement.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem('theme', theme);
    });
}

// Search functionality
function initSearchFunctionality() {
    const searchInput = document.querySelector('.search-input');
    if (!searchInput) return;
    
    searchInput.addEventListener('input', (e) => {
        const query = e.target.value.toLowerCase();
        const sections = document.querySelectorAll('section');
        
        sections.forEach(section => {
            const content = section.textContent.toLowerCase();
            if (content.includes(query) || query === '') {
                section.style.display = 'block';
            } else {
                section.style.display = 'none';
            }
        });
    });
}

// Utility: Debounce function
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

// Initialize smooth progress bar
function initProgressBar() {
    const progressBar = document.querySelector('.progress-bar');
    if (!progressBar) return;
    
    window.addEventListener('scroll', () => {
        const windowHeight = window.innerHeight;
        const documentHeight = document.documentElement.scrollHeight;
        const scrollTop = window.pageYOffset || document.documentElement.scrollTop;
        const scrollPercent = (scrollTop / (documentHeight - windowHeight)) * 100;
        
        progressBar.style.width = scrollPercent + '%';
    });
}

// Initialize active nav link highlighting
function initActiveNavLinks() {
    const sections = document.querySelectorAll('section[id]');
    const navLinks = document.querySelectorAll('.nav-links a[href^="#"]');
    
    if (sections.length === 0 || navLinks.length === 0) return;
    
    window.addEventListener('scroll', () => {
        const scrollY = window.pageYOffset;
        
        sections.forEach(section => {
            const sectionTop = section.offsetTop - 100;
            const sectionHeight = section.offsetHeight;
            const sectionId = section.getAttribute('id');
            
            if (scrollY >= sectionTop && scrollY < sectionTop + sectionHeight) {
                navLinks.forEach(link => {
                    link.classList.remove('active');
                    if (link.getAttribute('href') === `#${sectionId}`) {
                        link.classList.add('active');
                    }
                });
            }
        });
    });
}

// Add parallax effect to hero
window.addEventListener('scroll', () => {
    const hero = document.querySelector('.hero');
    if (hero) {
        const scrolled = window.pageYOffset;
        hero.style.transform = `translateY(${scrolled * 0.5}px)`;
        hero.style.opacity = 1 - (scrolled / 800);
    }
});

// Console Easter Egg
console.log('%c🚀 ReproVM', 'font-size: 20px; font-weight: bold; color: #2563eb;');
console.log('%cContent-Addressed Task Execution VM', 'font-size: 14px; color: #64748b;');
console.log('%cBuilt with C99, SHA-256, and pthreads', 'font-size: 12px; color: #10b981;');
console.log('%c\nInterested in contributing? Check out: https://github.com/hoangsonww/ReproVM-Virtual-Machine', 'font-size: 11px; color: #7c3aed;');
