module.exports = {
    title: 'QtPromise',
    description: 'Promises/A+ implementation for Qt/C++',
    dest: 'dist/docs',
    head: [
        ['link', { rel: 'icon', href: `/favicon.png` }],
    ],
    plugins: [
        ['@vuepress/google-analytics', {
            ga: 'UA-113899811-1'
        }]
    ],
    themeConfig: {
        repo: 'simonbrunel/qtpromise',
        lastUpdated: 'Last Updated',
        smoothScroll: false,
        editLinks: true,
        sidebarDepth: 2,
        docsDir: 'docs',
        algolia: {
            apiKey: '0e6e9cccb8c2c360a5543e28c4e31cb8',
            indexName: 'qtpromise'
        },
        nav: [
            { text: 'Home', link: '/' },
            { text: 'Guide', link: '/qtpromise/getting-started' },
            { text: 'API Reference', link: '/qtpromise/api-reference' },
        ],
        sidebar: [
            '/qtpromise/getting-started',
            '/qtpromise/qtconcurrent',
            '/qtpromise/qtsignals',
            '/qtpromise/thread-safety',
            {
                title: 'API Reference',
                path: '/qtpromise/api-reference',
                children: [
                    //['/qtpromise/api-reference', 'Overview'],
                    {
                        title: 'QPromise',
                        children: [
                            '/qtpromise/qpromise/constructor',
                            '/qtpromise/qpromise/convert',
                            '/qtpromise/qpromise/delay',
                            '/qtpromise/qpromise/each',
                            '/qtpromise/qpromise/fail',
                            '/qtpromise/qpromise/filter',
                            '/qtpromise/qpromise/finally',
                            '/qtpromise/qpromise/isfulfilled',
                            '/qtpromise/qpromise/ispending',
                            '/qtpromise/qpromise/isrejected',
                            '/qtpromise/qpromise/map',
                            '/qtpromise/qpromise/reduce',
                            '/qtpromise/qpromise/tap',
                            '/qtpromise/qpromise/tapfail',
                            '/qtpromise/qpromise/then',
                            '/qtpromise/qpromise/timeout',
                            '/qtpromise/qpromise/wait',
                            '/qtpromise/qpromise/reject',
                            '/qtpromise/qpromise/resolve'
                        ]
                    },
                    {
                        title: 'Helpers',
                        children: [
                            '/qtpromise/helpers/all',
                            '/qtpromise/helpers/attempt',
                            '/qtpromise/helpers/connect',
                            '/qtpromise/helpers/each',
                            '/qtpromise/helpers/filter',
                            '/qtpromise/helpers/map',
                            '/qtpromise/helpers/reduce',
                            '/qtpromise/helpers/resolve'
                        ]
                    },
                    {
                        title: 'Exceptions',
                        children: [
                            '/qtpromise/exceptions/canceled',
                            '/qtpromise/exceptions/context',
                            '/qtpromise/exceptions/conversion',
                            '/qtpromise/exceptions/timeout',
                            '/qtpromise/exceptions/undefined'
                        ]
                    }
                ]
            }
        ]
    }
}
