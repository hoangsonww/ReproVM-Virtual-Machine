# ReproVM Wiki

This directory contains the **official ReproVM wiki** - a professional, interactive web documentation site.

## 🌐 What's Included

- **`index.html`** - Main wiki page with complete documentation
- **`packages/styles.css`** - Professional dark theme styling
- **`packages/script.js`** - Interactive features and animations
- **`serve_wiki.sh`** - Simple local server script

## 🚀 Quick Start

### View Locally

```bash
# Start the server (default port 8000)
./serve_wiki.sh

# Or specify a custom port
./serve_wiki.sh 3000
```

Then open your browser to: **http://localhost:8000**

### Deploy to GitHub Pages

```bash
# The index.html is already at the root, so just enable GitHub Pages
# Go to: Repository Settings → Pages → Source: main branch → Root → Save
```

Your wiki will be live at: `https://yourusername.github.io/ReproVM-Virtual-Machine/`

### Deploy with Docker

```bash
# Build and run
docker run -d -p 8080:80 -v $(pwd):/usr/share/nginx/html:ro nginx:alpine

# Access at http://localhost:8080
```

## 📋 Features

### Documentation Coverage
- ✅ Complete project overview
- ✅ Feature showcase with visual cards
- ✅ Interactive Mermaid diagrams
- ✅ Architecture deep-dive
- ✅ Quick start guide
- ✅ CLI usage reference
- ✅ Real-world examples
- ✅ Advanced features
- ✅ Contributing guidelines
- ✅ FAQ section
- ✅ Troubleshooting guide
- ✅ Technology stack details

### Interactive Features
- 📱 Fully responsive design
- 🎨 Professional dark theme
- 📋 Copy-to-clipboard for code blocks
- 🔗 Smooth scroll navigation
- ✨ Scroll-based animations
- 📊 Mermaid diagram rendering
- 🎯 Active navigation highlighting
- 🌊 Parallax hero effect

### Design Highlights
- Modern gradient accents
- Clean typography with Inter & Fira Code
- Glassmorphism effects
- Smooth transitions and hover states
- Professional color scheme
- Optimized for readability

## 🎨 Customization

### Colors

Edit `packages/styles.css` to change the theme:

```css
:root {
    --primary-color: #2563eb;      /* Main accent */
    --secondary-color: #7c3aed;    /* Secondary accent */
    --dark-bg: #0f172a;            /* Background */
    --card-bg: #1e293b;            /* Card background */
    --text-primary: #f8fafc;       /* Main text */
}
```

### Content

Edit `index.html` to update:
- Hero section text
- Feature descriptions
- Code examples
- Documentation sections
- Footer links

### Scripts

Edit `packages/script.js` to customize:
- Smooth scroll behavior
- Copy button functionality
- Animation triggers
- Theme toggle (if enabled)

## 📦 Technology Stack

- **HTML5** - Semantic markup
- **CSS3** - Modern styling with custom properties
- **JavaScript (ES6+)** - Interactive features
- **Mermaid.js** - Diagram rendering
- **Google Fonts** - Inter & Fira Code typography

## 🔧 Development

### File Structure

```
ReproVM-Virtual-Machine/
├── index.html          # Main wiki page
├── packages/
│   ├── styles.css      # Styling
│   └── script.js       # Interactivity
├── serve_wiki.sh       # Local server script
└── WIKI.md            # This file
```

### Local Development

1. Edit files directly
2. Refresh browser to see changes
3. No build step required!

### Adding New Sections

```html
<section id="new-section">
    <h2 class="section-title">New Section</h2>
    <p class="section-subtitle">Description</p>
    
    <div class="features-grid">
        <div class="feature-card">
            <h3>Feature</h3>
            <p>Description</p>
        </div>
    </div>
</section>
```

### Adding Mermaid Diagrams

```html
<div class="mermaid-container">
    <div class="mermaid">
        graph LR
            A[Start] --> B[End]
    </div>
</div>
```

## 🚢 Deployment Options

### 1. GitHub Pages (Recommended)

```bash
# Already configured! Just enable in repo settings
# Settings → Pages → Source: main → Root → Save
```

### 2. Netlify

```bash
# Create netlify.toml
[build]
  publish = "."

# Deploy
netlify deploy --prod
```

### 3. Vercel

```bash
# Deploy with Vercel CLI
vercel --prod
```

### 4. Static Hosting

Upload `index.html` and `packages/` to any web server.

## 📖 Usage Examples

### Link from README

Add this to your main README.md:

```markdown
## 📚 Documentation

**[📖 View Full Wiki →](https://yourusername.github.io/ReproVM-Virtual-Machine/)**

For comprehensive documentation, examples, and guides, visit our interactive wiki.
```

### Embed in CI

```yaml
# .github/workflows/deploy-wiki.yml
name: Deploy Wiki
on:
  push:
    branches: [main]
    paths:
      - 'index.html'
      - 'packages/**'
jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: .
```

## 🎯 Best Practices

1. **Keep it Updated** - Sync wiki with README changes
2. **Test Locally** - Always test with `./serve_wiki.sh` before deploying
3. **Validate HTML** - Use W3C validator
4. **Optimize Images** - If you add images, compress them
5. **Mobile First** - Test on different screen sizes
6. **Accessibility** - Maintain semantic HTML and ARIA labels

## 🐛 Troubleshooting

### Server won't start

```bash
# Ensure you have Python 3 installed
python3 --version

# Or install Node.js and http-server
npm install -g http-server
```

### Mermaid diagrams not rendering

- Check browser console for errors
- Ensure internet connection (CDN dependency)
- Clear browser cache

### Styles not loading

- Check file paths in `index.html`
- Ensure `packages/` directory exists
- Check browser console for 404 errors

## 📝 License

This wiki is part of the ReproVM project and is licensed under the MIT License.

## 🤝 Contributing

Improvements to the wiki are welcome! Please:

1. Test locally with `./serve_wiki.sh`
2. Ensure responsive design
3. Maintain consistent styling
4. Update this README if needed

---

**Built with ❤️ for the ReproVM community**
