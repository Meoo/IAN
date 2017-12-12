
const path = require('path');
const CleanWebpackPlugin = require('clean-webpack-plugin');
const ExtractTextPlugin = require('extract-text-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  context: __dirname,

  entry: {
    loader: './src/loader.js',
    ian: './src/ian.js'
  },

  plugins: [
    new CleanWebpackPlugin([path.resolve('@CMAKE_INSTALL_PREFIX@/client/assets')]),
    new ExtractTextPlugin('[name].[contenthash].css'),
    new HtmlWebpackPlugin({
      title: 'IAN',
      filename: global.ian_dev ? "index.html" : '../ian.html',
      chunks: ['loader'],
      template: 'src/shell.html',
      minify: global.ian_prod ? {
        collapseWhitespace: true
      } : null
    })
  ],

  target: "web",

  output: {
    path: path.resolve('@CMAKE_INSTALL_PREFIX@/client/assets'),
    publicPath: global.ian_dev ? "" : "/assets/",
    filename: '[name].[chunkhash].js'
  },

  module: {
    rules: [
      {
        test: /\.css$/,
        use: ExtractTextPlugin.extract({
          fallback: "style-loader",
          use: "css-loader"
        })
      },
      {
        test: /\.png$/,
        loader: 'file-loader'
      }
    ]
  },

  resolve: {
    extensions: ['.js', '.json', '.ts'],
    alias: {@IAN_GEN_WP_ALIASES@
    }
  }
};
