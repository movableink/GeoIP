var warning = require('debug')('geoip:test:country6:warning');
var semver = require('semver');
var randomip = require('random-ip');
// memwatch only works with node 0.6.x ~ 0.10.x
if (semver.gte(process.version, '0.6.0') && semver.lt(process.version, '0.11.0')) {
    require('memwatch').on('leak', function(info) {
        warning('Memory leak detected: %j', info);
    });
}

var path = require('path');
var mocha = require('mocha');
var chai = require('chai');
chai.Assertion.includeStack = true;

var should = chai.should();

var geoip = require('../index.js');
var Country6 = geoip.Country6;
var file = path.resolve(__dirname, '../database/GeoIPv6.dat');

describe('Country6', function() {
    describe('Instance', function() {
        var instance = new Country6(file, true);

        it('should be a object', function(done) {
            instance.should.be.an('object');
            setTimeout(done, 1);
        });

        it('should has a lookup method', function(done) {
            instance.lookup.should.be.a('function');
            setTimeout(done, 1);
        });

        it('should has a lookupSync method', function(done) {
            instance.lookupSync.should.be.a('function');
            setTimeout(done, 1);
        });

        it('should has a update method', function(done) {
            instance.update.should.be.a('function');
            setTimeout(done, 1);
        });

        describe('Synchrouns Lookup', function() {
            it('should throw error when input is not a string', function(done) {
                try {
                    instance.lookupSync(null);
                } catch(err) {
                    should.exist(err);
                    setTimeout(done, 1);
                }
            });

            // Test file doesn't have any IPs that have associated DNS name, travis can't stub hosts
            it.skip('should find location by domain', function(done) {
                var data = instance.lookupSync('www.google.com');
                data.should.be.a('object');
                setTimeout(done, 1);
            });

            it('should find location by ip address', function(done) {
                var data = instance.lookupSync('2001:200::');
                data.should.be.a('object');
                setTimeout(done, 1);
            });
        });

        describe('Asynchrouns Lookup', function() {
            it('should return error when input is not a string', function(done) {
                instance.lookup(null, function(err, data) {
                    should.exist(err);
                    setTimeout(done, 1);
                });
            });

            // Test file doesn't have any IPs that have associated DNS name, travis can't stub hosts
            it.skip('should find location by domain', function(done) {
                instance.lookup('www.google.com', function(err, data) {
                    should.not.exist(err);
                    should.exist(data);
                    data.should.be.an('object');
                    setTimeout(done, 1);
                });
            });

            it('should find location by ip address', function(done) {
                instance.lookup('2001:200::', function(err, data) {
                    should.not.exist(err);
                    should.exist(data);
                    data.should.be.an('object');
                    setTimeout(done, 1);
                });
            });

            describe('Test random IPs', function() {
              it('should not crash for 2000 v6 IPs', function (done) {
                for (var i = 0; i < 2000; ++i) {
                  var ip = randomip(':', 0);
                  //console.log(ip);
                  var addr = instance.lookupSync(ip);
                  //console.log(addr);
                }
                  setTimeout(done, 1);
              });
            });

            describe('Update database on the fly', function() {
                it('should be ok', function(done) {
                    instance.update(file).should.be.ok;
                    setTimeout(done, 1);
                });
            });
        });
    });

    describe('Silent instance', function() {
        var instance = new Country6(file, {silent: true});
        describe('Synchrouns Lookup', function() {
            it('should return null when input is not a string', function(done) {
                should.equal(instance.lookupSync(null), null);
                setTimeout(done, 1);
            });
        });

        describe('Asynchrouns Lookup', function() {
            it('should return null when input is not a string', function(done) {
                instance.lookup(null, function(err, data) {
                    should.not.exist(err);
                    should.equal(data, null);
                    setTimeout(done, 1);
                });
            });
        });
    });
});
