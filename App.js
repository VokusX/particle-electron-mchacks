import React, { Component } from 'react'
import { Alert, TouchableOpacity, Slider, View, Text, ActivityIndicator, ScrollView, StyleSheet, Image } from 'react-native';
//import { Card } from 'react-native-elements' // 0.19.0
import * as firebase from 'firebase'; // 4.9.1
import "@expo/vector-icons"; // 6.2.2
const firebaseConfig = {
  apiKey: "PyyPc0E1o2I1Z4Hp2f2zgJQFZ3RUdZSXDnq5vl6r",
  authDomain: "<your-auth-domain>",
  databaseURL: "https://we-cu-fbf27.firebaseio.com/",
  storageBucket: "<your-storage-bucket>",
};
if (firebase.apps.length === 0) {
  firebase.initializeApp(firebaseConfig)
}
const thresholds = {
  "flood": 1,
  "humidity": 15,
  "loudness": 205,
  "motion": 1,
  "temp": 30,
  "power": 1
}

export default class App extends Component {
  constructor(props) {
    super(props)
    this.itemsRef = firebase.database().ref();
    this.state = {
      loading: true,
      error: false,
      status: [],
    }
  }

  componentWillMount = async () => {
    try {
      let status = []
      this.itemsRef.on("value", (snap) => {
        snap.forEach((child) => {
          status = [
            {"name": "flood", "value": child.val().data.flood, "img": "https://cdn.iconscout.com/public/images/icon/free/png-256/flood-symbol-alarm-attention-board-error-warning-355890908d0c4ae5-256x256.png"},
            {"name": "smoke", "value": child.val().data.gas, "img": "http://www.iconsplace.com/download/yellow-gas-256.gif"},
            {"name": "humidity", "value": child.val().data.humidity, "img": "http://icons.iconarchive.com/icons/custom-icon-design/lovely-weather-2/256/Humidity-icon.png"},
            {"name": "loudness", "value": child.val().data.loudness, "img": "http://icons.iconarchive.com/icons/custom-icon-design/mono-general-4/256/sound-icon.png"},
            {"name": "motion", "value": child.val().data.motion, "img": "https://cdn3.iconfinder.com/data/icons/joe-pictos-business-bold/100/transfer_bold_convert-256.png"},
            {"name": "temp", "value": child.val().data.temp, "img": "https://www.iconsdb.com/icons/preview/red/temperature-2-xxl.png"},
          ]
        })
      })
      this.setState({loading: false, status})
    } catch (e) {
      console.log("e")
      this.setState({loading: false, error: true})
    }
  }
  
  _onPressButton = () => {
  }
  renderPost = ({name, value, img}) => {
    return (
      <TouchableOpacity onPress={this._onPressButton}
        key={name}
        style={styles.property}
      >
        <View style={styles.postNumber}>
          <Image
            source={{ uri: img }}
            style={{ height: 42, width: 42 }}
          />
        </View>
        <View style={styles.propertyContent}>
          <Text style={styles.propertyName}>
            {name}
          </Text>
          <Text style={styles.propertyValue}>
            { value >= thresholds[name] ? <Text>Unsafe</Text> : <Text style={{color: "green"}}>Safe</Text> }
          </Text>
        </View>
      </TouchableOpacity>
    )
  }

  render() {
    const {status, loading, error} = this.state

    if (loading) {
      return (
        <View style={styles.center}>
          <ActivityIndicator animating={true} />
        </View>
      )
    }

    if (error) {
      return (
        <View style={styles.center}>
          <Text>
            Failed to load status!
          </Text>
        </View>
      )
    }

    return (
      <ScrollView style={styles.container}>
        {status.map(this.renderPost)}
      </ScrollView>
    )
  }
}

const styles = StyleSheet.create({
  container: {
    top: 40,
    width: '100%',
    flex: 1,
    padding: 10,
    backgroundColor: "#E8EAF6"
  },
  property: {
    flexDirection: 'row',
    backgroundColor: 'white',
    marginTop: 10,
    borderRadius: 10,
    paddingLeft: 10
  },
  postNumber: {
    width: 50,
    justifyContent: 'center',
    alignItems: 'center',
  },
  propertyContent: {
    flex: 1,
    flexDirection: 'row',
    marginLeft: 10,
    paddingVertical: 25,
    paddingRight: 15,
  },
  propertyName: {
    fontWeight: "100",
    color: 'black',
    fontSize: '30%'
  },
  propertyValue: {
    fontWeight: "100",
    color: '#F44336',
    position: 'absolute',
    right: "5%%",
    fontSize: "50%"
  },
  center: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
})