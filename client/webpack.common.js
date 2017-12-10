
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
    new CleanWebpackPlugin(['dist']),
    new ExtractTextPlugin('[name].css'),
    new HtmlWebpackPlugin({
      title: 'IAN',
      chunks: ['loader'],
      template: 'src/shell.html',
      minify: {
        collapseWhitespace: !!global.ian_prod
      }
    })
  ],

  target: "web",

  externals: {
    jquery: 'jQuery'
  },

  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: '[name].js'
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
