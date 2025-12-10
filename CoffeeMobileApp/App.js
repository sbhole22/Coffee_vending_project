import 'react-native-gesture-handler';
import React, { useState, useEffect } from 'react';
import { StyleSheet, Text, View, TouchableOpacity, ScrollView, Dimensions, Alert, Image } from 'react-native';
import { StatusBar } from 'expo-status-bar';
import { GestureHandlerRootView, PanGestureHandler } from 'react-native-gesture-handler';
import Animated, { useAnimatedStyle, useSharedValue, withSpring, runOnJS, useAnimatedGestureHandler } from 'react-native-reanimated';
import { MaterialCommunityIcons } from '@expo/vector-icons';
import { database } from './firebaseConfig';
import { ref, set, onValue } from 'firebase/database';

const { width, height } = Dimensions.get('window');

// --- Constants & Data ---
const INGREDIENTS = [
  { id: 'espresso', name: 'Espresso', color: '#3C2A21' },
  { id: 'milk_whole', name: 'Whole Milk', color: '#F5EBE0' },
  { id: 'milk_almond', name: 'Almond Milk', color: '#EED9C4' },
  { id: 'milk_oat', name: 'Oat Milk', color: '#E0D8C0' },
  { id: 'foam', name: 'Foam', color: '#FFFFFF' },
  { id: 'water', name: 'Water', color: '#AEC6CF' },
  { id: 'syrup', name: 'Syrup', color: '#D2691E' },
  { id: 'ice', name: 'Ice', color: '#E0F7FA' },
];

const PRESETS = [
  { name: 'Espresso', ingredients: ['espresso'] },
  { name: 'Latte', ingredients: ['espresso', 'milk', 'foam'] },
  { name: 'Cappuccino', ingredients: ['espresso', 'milk', 'foam'] }, // More foam usually
  { name: 'Americano', ingredients: ['espresso', 'water'] },
  { name: 'Macchiato', ingredients: ['espresso', 'foam'] },
];

const MENU_ITEMS = [
  { id: 'miy', name: 'Make it yours', price: '6.00', icon: 'flask-outline', color: '#00D2FF' },
  { id: 'espresso', name: 'Espresso', price: '2.99', icon: 'coffee', color: '#C0C0C0' },
  { id: 'americano', name: 'Americano', price: '3.99', icon: 'coffee-outline', color: '#C0C0C0' },
  { id: 'latte', name: 'Latte', price: '4.99', icon: 'cup', color: '#C0C0C0' },
];

const IMAGES = {
  miy: require('./assets/make_it_yours.png'),
  espresso: require('./assets/espresso.png'),
  americano: require('./assets/americano.png'),
  latte: require('./assets/latte.png'),
};

const MENU_ITEMS_WITH_IMAGES = [
  { id: 'miy', name: 'Make it yours', price: '6.00', image: IMAGES.miy, color: '#00D2FF' },
  { id: 'espresso', name: 'Espresso', price: '2.99', image: IMAGES.espresso, color: '#C0C0C0' },
  { id: 'americano', name: 'Americano', price: '3.99', image: IMAGES.americano, color: '#C0C0C0' },
  { id: 'latte', name: 'Latte', price: '4.99', image: IMAGES.latte, color: '#C0C0C0' },
];

// --- Components ---

// Splash Screen Component
const SplashScreen = ({ onStart }) => {
  return (
    <View style={styles.splashContainer}>
      <View style={styles.splashContent}>
        <Text style={styles.splashTitle}>AuraMist Cafe</Text>
        <Text style={styles.splashSubtitle}>Be your own Barista !</Text>
        
        <View style={styles.splashImages}>
           <Image source={IMAGES.latte} style={styles.splashImageHero} />
           <View style={styles.splashImageRow}>
             <Image source={IMAGES.espresso} style={styles.splashImageSecondary} />
             <Image source={IMAGES.americano} style={styles.splashImageSecondary} />
           </View>
        </View>
      </View>

      <View style={styles.splashButtonContainer}>
        <TouchableOpacity style={styles.splashButton} onPress={onStart}>
          <MaterialCommunityIcons name="play" size={40} color="#101024" />
        </TouchableOpacity>
        <Text style={styles.splashButtonText}>Get started</Text>
      </View>
    </View>
  );
};

// Menu Screen Component
const MenuScreen = ({ onSelect, onBack, pendingOrders }) => {
  return (
    <View style={styles.menuContainer}>
      <TouchableOpacity onPress={onBack} style={styles.absoluteBackBtn}>
           <MaterialCommunityIcons name="arrow-left" size={30} color="#FFFFFF" />
      </TouchableOpacity>

      <View style={styles.menuHeader}>
        <Text style={styles.menuTitle}>Select your drink</Text>
      </View>

      <ScrollView 
        contentContainerStyle={styles.menuListContent}
      >
        {MENU_ITEMS_WITH_IMAGES.map((item) => (
          <TouchableOpacity key={item.id} style={styles.menuListItem} onPress={() => onSelect(item)}>
            <View style={styles.menuListIcon}>
               <Image source={item.image} style={styles.menuItemImage} />
            </View>
            <View style={styles.menuListInfo}>
                <Text style={styles.menuListName}>{item.name}</Text>
                <Text style={styles.menuListPrice}>${item.price}</Text>
            </View>
            <MaterialCommunityIcons name="chevron-right" size={30} color="#555" />
          </TouchableOpacity>
        ))}
      </ScrollView>

      {/* Pending Orders Tab */}
      {pendingOrders.length > 0 && (
          <View style={styles.pendingOrdersContainer}>
              <Text style={styles.pendingOrdersTitle}>Pending Orders</Text>
              <ScrollView horizontal showsHorizontalScrollIndicator={false}>
                  {pendingOrders.map((order, index) => (
                      <View key={index} style={styles.pendingOrderCard}>
                          <Text style={styles.pendingOrderId}>#{order.id}</Text>
                          <Text style={styles.pendingOrderName}>{order.item}</Text>
                          <Text style={styles.pendingOrderStatus}>Preparing...</Text>
                      </View>
                  ))}
              </ScrollView>
          </View>
      )}
    </View>
  );
};

// Draggable Ingredient Component
const DraggableIngredient = ({ ingredient, onDrop }) => {
  const translateX = useSharedValue(0);
  const translateY = useSharedValue(0);

  const gestureHandler = useAnimatedGestureHandler({
    onStart: (_, ctx) => {
      ctx.startX = translateX.value;
      ctx.startY = translateY.value;
    },
    onActive: (event, ctx) => {
      translateX.value = ctx.startX + event.translationX;
      translateY.value = ctx.startY + event.translationY;
    },
    onEnd: (event) => {
      // Simple collision detection logic (approximate drop zone)
      // Assuming drop zone is roughly in the middle of the screen vertically
      if (event.absoluteY > 200 && event.absoluteY < 500) {
        runOnJS(onDrop)(ingredient);
      }
      translateX.value = withSpring(0);
      translateY.value = withSpring(0);
    },
  });

  const animatedStyle = useAnimatedStyle(() => {
    return {
      transform: [
        { translateX: translateX.value },
        { translateY: translateY.value },
      ],
      zIndex: translateX.value !== 0 ? 100 : 1,
    };
  });

  return (
    <PanGestureHandler onGestureEvent={gestureHandler}>
      <Animated.View style={[styles.ingredientItem, animatedStyle, { backgroundColor: ingredient.color }]}>
        <Text style={styles.ingredientText}>{ingredient.name}</Text>
      </Animated.View>
    </PanGestureHandler>
  );
};

const CupLayer = ({ ingredient, index, onUpdateLevel, onRemove }) => {
  const flex = useSharedValue(ingredient.level || 1);
  const translateX = useSharedValue(0);
  const opacity = useSharedValue(1);

  // Sync shared value if prop changes
  useEffect(() => {
      flex.value = ingredient.level || 1;
  }, [ingredient.level]);

  const gestureHandler = useAnimatedGestureHandler({
    onStart: (_, ctx) => {
      ctx.startFlex = flex.value;
    },
    onActive: (event, ctx) => {
      // Dragging UP (negative Y) increases size
      const change = -event.translationY / 50; 
      flex.value = Math.max(0.2, ctx.startFlex + change);
      
      translateX.value = event.translationX;
      if (Math.abs(event.translationX) > 80) {
          opacity.value = 0.5;
      } else {
          opacity.value = 1;
      }
    },
    onEnd: (event) => {
      if (Math.abs(event.translationX) > 100) {
          translateX.value = withSpring(Math.sign(event.translationX) * 500, {}, () => {
              runOnJS(onRemove)(index);
          });
      } else {
          translateX.value = withSpring(0);
          opacity.value = withSpring(1);
          runOnJS(onUpdateLevel)(index, flex.value);
      }
    },
  });

  const animatedStyle = useAnimatedStyle(() => {
    return {
      flex: flex.value,
      transform: [{ translateX: translateX.value }],
      opacity: opacity.value,
    };
  });

  return (
    <PanGestureHandler onGestureEvent={gestureHandler} activeOffsetX={[-20, 20]} activeOffsetY={[-20, 20]}>
      <Animated.View style={[styles.cupLayer, { backgroundColor: ingredient.color }, animatedStyle]} />
    </PanGestureHandler>
  );
};

// Tic-Tac-Toe Game Component Removed

export default function App() {
  const [currentScreen, setCurrentScreen] = useState('splash'); // splash, menu, custom, standard
  const [selectedCoffee, setSelectedCoffee] = useState(null);
  const [cupIngredients, setCupIngredients] = useState([]);
  const [availableIngredients, setAvailableIngredients] = useState(INGREDIENTS);
  const [mood, setMood] = useState("Start Brewing");
  const [size, setSize] = useState(1); // 0: Small, 1: Medium, 2: Large
  const [strength, setStrength] = useState(1); // 0: Light, 1: Medium, 2: Dark
  const [temperature, setTemperature] = useState(2); // 0: Cold, 1: Warm, 2: Hot
  const [orderId, setOrderId] = useState(null);
  const [pendingOrders, setPendingOrders] = useState([]);

  // Listen for order status changes
  useEffect(() => {
      if (!orderId) return;
      
      const orderRef = ref(database, 'orders/' + orderId);
      const unsubscribe = onValue(orderRef, (snapshot) => {
          const data = snapshot.val();
          if (data && (data.status === 'completed' || data.status === 'ready')) {
              Alert.alert("Order Ready!", "Your drink is ready for pickup!");
              // Remove from pending orders
              setPendingOrders(prev => prev.filter(o => o.id !== orderId));
              setOrderId(null);
              setCurrentScreen('menu');
          }
      });
      
      return () => unsubscribe();
  }, [orderId]);

  const handleDrop = (ingredient) => {
    setCupIngredients((prev) => [...prev, { ...ingredient, level: 1 }]);
    setAvailableIngredients((prev) => prev.filter(i => i.id !== ingredient.id));
  };

  const updateLevel = (index, newLevel) => {
    setCupIngredients(prev => {
        const newArr = [...prev];
        newArr[index] = { ...newArr[index], level: newLevel };
        return newArr;
    });
  };

  const removeIngredient = (index) => {
      const item = cupIngredients[index];
      setCupIngredients(prev => prev.filter((_, i) => i !== index));
      // Find original ingredient data to add back
      const original = INGREDIENTS.find(i => i.id === item.id);
      if (original) {
          setAvailableIngredients(prev => [...prev, original]);
      }
  };

  const clearCup = () => {
    setCupIngredients([]);
    setAvailableIngredients(INGREDIENTS);
  };

  const handleMenuSelect = (item) => {
      setSelectedCoffee(item);
      if (item.id === 'miy') {
          setCurrentScreen('custom');
          setAvailableIngredients(INGREDIENTS);
          setCupIngredients([]);
      } else {
          setCurrentScreen('standard');
      }
  };

  const handleCheckout = () => {
      const newOrderId = Math.floor(Math.random() * 10000) + 1000;
      setOrderId(newOrderId);
      setPendingOrders(prev => [...prev, { id: newOrderId, item: selectedCoffee?.name }]);
      
      // Send order to Firebase
      const orderData = {
          id: newOrderId,
          item: selectedCoffee?.name,
          price: selectedCoffee?.price,
          size: ['Small', 'Medium', 'Large'][size],
          strength: ['Light', 'Medium', 'Dark'][strength],
          temperature: ['Cold', 'Warm', 'Hot'][temperature],
          ingredients: cupIngredients.map(ing => ({
              name: ing.name,
              level: ing.level
          })),
          status: 'pending',
          timestamp: Date.now()
      };

      set(ref(database, 'orders/' + newOrderId), orderData)
        .then(() => {
            console.log('Order sent to Firebase successfully!');
        })
        .catch((error) => {
            console.error('Error sending order: ', error);
            Alert.alert("Error", "Could not send order to the cloud. Please try again.");
        });

      // Reset custom state
      setCupIngredients([]);
      setAvailableIngredients(INGREDIENTS);
  };

  // Update Mood based on ingredients
  useEffect(() => {
    if (cupIngredients.length === 0) {
      setMood("Start Brewing");
      return;
    }
    
    const counts = {};
    cupIngredients.forEach(i => counts[i.id] = (counts[i.id] || 0) + 1);

    if (counts['syrup'] > 1) setMood("Sweet Tooth");
    else if (counts['espresso'] > 2) setMood("Power Up");
    else if (counts['milk'] > 2) setMood("Milky Way");
    else if (counts['ice'] > 0) setMood("Chill Vibes");
    else setMood("Barista Mode");

  }, [cupIngredients]);

  const renderCup = () => {
    // Visual representation of the cup filling up
    // Inverted glass: Last item entered is at the top (visually)
    // We use flexDirection: 'column-reverse' to stack from bottom up
    return (
      <View style={styles.cupContainer}>
        <View style={[styles.cup, { height: 150 + (size * 30), flexDirection: 'column-reverse' }]}>
          {cupIngredients.map((ing, index) => (
            <CupLayer 
                key={ing.id} // Use ID as key to maintain state during reorders if any
                ingredient={ing} 
                index={index} 
                onUpdateLevel={updateLevel}
                onRemove={removeIngredient}
            />
          ))}
          {cupIngredients.length === 0 && <Text style={styles.cupPlaceholder}>Drag Ingredients Here</Text>}
        </View>
        <Text style={styles.moodLabel}>{mood}</Text>
      </View>
    );
  };

  if (currentScreen === 'splash') {
    return (
      <GestureHandlerRootView style={styles.container}>
        <StatusBar style="light" />
        <SplashScreen onStart={() => setCurrentScreen('menu')} />
      </GestureHandlerRootView>
    );
  }

  if (orderId) {
      return (
        <View style={styles.orderContainer}>
            <StatusBar style="light" />
            <MaterialCommunityIcons name="check-circle-outline" size={100} color="#00FF88" />
            <Text style={styles.orderTitle}>Order Placed!</Text>
            <Text style={styles.orderId}>#{orderId}</Text>
            <Text style={styles.orderSubtitle}>Preparing your {selectedCoffee?.name}...</Text>
            <TouchableOpacity style={styles.homeButton} onPress={() => { setOrderId(null); setCurrentScreen('menu'); }}>
                <Text style={styles.homeButtonText}>Back to Menu</Text>
            </TouchableOpacity>
        </View>
      );
  }

  if (currentScreen === 'menu') {
      return (
        <GestureHandlerRootView style={styles.container}>
            <StatusBar style="light" />
            <MenuScreen onSelect={handleMenuSelect} onBack={() => setCurrentScreen('splash')} pendingOrders={pendingOrders} />
        </GestureHandlerRootView>
      );
  }

  return (
    <GestureHandlerRootView style={styles.container}>
      <StatusBar style="light" />
      
      <TouchableOpacity onPress={() => setCurrentScreen('menu')} style={styles.absoluteBackBtn}>
             <MaterialCommunityIcons name="arrow-left" size={30} color="#FFFFFF" />
      </TouchableOpacity>

      {/* Header */}
      <View style={styles.header}>
        <Text style={styles.headerTitle}>{selectedCoffee?.name || 'Customize'}</Text>
      </View>

      <ScrollView contentContainerStyle={styles.scrollContent}>
        
        {currentScreen === 'custom' ? (
          <>
            {/* Cup Area */}
            {renderCup()}

            {/* Strength & Temp Controls for Custom */}
             <View style={styles.controlGroup}>
                 <Text style={styles.controlLabel}>Strength</Text>
                 <View style={styles.segmentControl}>
                     {['Light', 'Medium', 'Dark'].map((opt, i) => (
                         <TouchableOpacity 
                            key={i} 
                            style={[styles.segmentBtn, strength === i && styles.segmentBtnActive]}
                            onPress={() => setStrength(i)}
                         >
                             <Text style={[styles.segmentText, strength === i ? {color:'#101024'} : {}]}>{opt}</Text>
                         </TouchableOpacity>
                     ))}
                 </View>
             </View>

             <View style={styles.controlGroup}>
                 <Text style={styles.controlLabel}>Temperature</Text>
                 <View style={styles.segmentControl}>
                     {['Cold', 'Warm', 'Hot'].map((opt, i) => (
                         <TouchableOpacity 
                            key={i} 
                            style={[styles.segmentBtn, temperature === i && styles.segmentBtnActive]}
                            onPress={() => setTemperature(i)}
                         >
                             <Text style={[styles.segmentText, temperature === i ? {color:'#101024'} : {}]}>{opt}</Text>
                         </TouchableOpacity>
                     ))}
                 </View>
             </View>

            {/* Size Slider (Simulated with buttons for simplicity in RN without extra libs) */}
            <View style={styles.sizeControl}>
                <Text style={styles.sectionTitle}>Size: {['Small', 'Medium', 'Large'][size]}</Text>
                <View style={styles.sizeButtons}>
                    {[0, 1, 2].map((s) => (
                        <TouchableOpacity 
                            key={s} 
                            style={[styles.sizeBtn, size === s && styles.activeSizeBtn]}
                            onPress={() => setSize(s)}
                        >
                            <Text style={[styles.sizeBtnText, size === s && styles.activeSizeBtnText]}>{['S', 'M', 'L'][s]}</Text>
                        </TouchableOpacity>
                    ))}
                </View>
            </View>

            {/* Ingredients List */}
            <Text style={styles.sectionTitle}>Ingredients</Text>
            <View style={styles.ingredientsContainer}>
              {availableIngredients.map((ing) => (
                <DraggableIngredient key={ing.id} ingredient={ing} onDrop={handleDrop} />
              ))}
            </View>

            <TouchableOpacity style={styles.clearButton} onPress={clearCup}>
                <Text style={styles.clearButtonText}>Clear Cup</Text>
            </TouchableOpacity>

            <TouchableOpacity style={styles.checkoutButton} onPress={handleCheckout}>
                 <Text style={styles.checkoutButtonText}>Checkout - ${selectedCoffee?.price}</Text>
            </TouchableOpacity>
          </>
        ) : (
          <View style={styles.standardContainer}>
             {/* Standard Customization Screen (Espresso, Latte, etc) */}
             <View style={styles.standardImageContainer}>
                <Image source={selectedCoffee?.image} style={styles.standardImage} />
             </View>
             
             <View style={styles.controlGroup}>
                 <Text style={styles.controlLabel}>Strength</Text>
                 <View style={styles.segmentControl}>
                     {['Light', 'Medium', 'Dark'].map((opt, i) => (
                         <TouchableOpacity 
                            key={i} 
                            style={[styles.segmentBtn, strength === i && styles.segmentBtnActive]}
                            onPress={() => setStrength(i)}
                         >
                             <Text style={[styles.segmentText, strength === i ? {color:'#101024'} : {}]}>{opt}</Text>
                         </TouchableOpacity>
                     ))}
                 </View>
             </View>

             <View style={styles.controlGroup}>
                 <Text style={styles.controlLabel}>Temperature</Text>
                 <View style={styles.segmentControl}>
                     {['Cold', 'Warm', 'Hot'].map((opt, i) => (
                         <TouchableOpacity 
                            key={i} 
                            style={[styles.segmentBtn, temperature === i && styles.segmentBtnActive]}
                            onPress={() => setTemperature(i)}
                         >
                             <Text style={[styles.segmentText, temperature === i ? {color:'#101024'} : {}]}>{opt}</Text>
                         </TouchableOpacity>
                     ))}
                 </View>
             </View>

             <View style={styles.controlGroup}>
                 <Text style={styles.controlLabel}>Size</Text>
                 <View style={styles.segmentControl}>
                     {['Small', 'Medium', 'Large'].map((opt, i) => (
                         <TouchableOpacity key={i} style={[styles.segmentBtn, size === i && styles.segmentBtnActive]} onPress={() => setSize(i)}>
                             <Text style={[styles.segmentText, size === i ? {color:'#101024'} : {}]}>{opt}</Text>
                         </TouchableOpacity>
                     ))}
                 </View>
             </View>

             <TouchableOpacity style={styles.checkoutButton} onPress={handleCheckout}>
                 <Text style={styles.checkoutButtonText}>Checkout - ${selectedCoffee?.price}</Text>
             </TouchableOpacity>
          </View>
        )}

      </ScrollView>
    </GestureHandlerRootView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#101024',
    paddingTop: 50,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'center',
    marginBottom: 20,
  },
  tab: {
    paddingVertical: 10,
    paddingHorizontal: 20,
    borderBottomWidth: 2,
    borderBottomColor: 'transparent',
  },
  activeTab: {
    borderBottomColor: '#3C2A21',
  },
  tabText: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#3C2A21',
  },
  scrollContent: {
    paddingBottom: 50,
    alignItems: 'center',
  },
  cupContainer: {
    alignItems: 'center',
    marginBottom: 30,
    height: 250,
    justifyContent: 'flex-end',
  },
  cup: {
    width: 120,
    borderBottomLeftRadius: 20,
    borderBottomRightRadius: 20,
    borderWidth: 4,
    borderColor: '#3C2A21',
    backgroundColor: '#fff',
    overflow: 'hidden',
    justifyContent: 'flex-end',
  },
  cupLayer: {
    width: '100%',
  },
  cupPlaceholder: {
    position: 'absolute',
    width: '100%',
    textAlign: 'center',
    top: '40%',
    color: '#ccc',
  },
  moodLabel: {
    marginTop: 10,
    fontSize: 20,
    fontWeight: 'bold',
    color: '#D2691E',
    fontStyle: 'italic',
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: '600',
    marginBottom: 10,
    marginTop: 20,
    color: '#333',
  },
  ingredientsContainer: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'center',
    gap: 10,
    width: '90%',
  },
  ingredientItem: {
    width: 80,
    height: 80,
    borderRadius: 40,
    justifyContent: 'center',
    alignItems: 'center',
    borderWidth: 1,
    borderColor: '#ddd',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  ingredientText: {
    fontWeight: 'bold',
    color: '#333',
  },
  sizeControl: {
      alignItems: 'center',
      width: '100%',
  },
  sizeButtons: {
      flexDirection: 'row',
      gap: 20,
  },
  sizeBtn: {
      width: 50,
      height: 50,
      borderRadius: 25,
      backgroundColor: '#eee',
      justifyContent: 'center',
      alignItems: 'center',
  },
  activeSizeBtn: {
      backgroundColor: '#3C2A21',
  },
  sizeBtnText: {
      fontSize: 16,
      color: '#333',
  },
  activeSizeBtnText: {
      color: '#fff',
  },
  clearButton: {
      marginTop: 20,
      padding: 10,
      backgroundColor: '#ff6b6b',
      borderRadius: 8,
  },
  clearButtonText: {
      color: '#fff',
      fontWeight: 'bold',
  },
  menuList: {
      width: '90%',
  },
  menuItem: {
      backgroundColor: '#fff',
      padding: 15,
      borderRadius: 10,
      marginBottom: 10,
      shadowColor: '#000',
      shadowOffset: { width: 0, height: 1 },
      shadowOpacity: 0.1,
      shadowRadius: 2,
      elevation: 2,
  },
  menuItemText: {
      fontSize: 18,
      fontWeight: 'bold',
      color: '#3C2A21',
  },
  menuItemSub: {
      fontSize: 12,
      color: '#888',
  },

  // --- Splash Screen Styles ---
  splashContainer: {
    flex: 1,
    backgroundColor: '#101024',
    padding: 30,
    justifyContent: 'space-between',
  },
  splashContent: {
    flex: 1,
    justifyContent: 'center',
  },
  splashTitle: {
    fontSize: 42,
    fontWeight: 'bold',
    color: '#FFFFFF',
    marginBottom: 10,
  },
  splashSubtitle: {
    fontSize: 24,
    color: '#FFFFFF',
    marginBottom: 20,
  },
  splashImages: {
    alignItems: 'center',
    marginTop: 10,
  },
  splashImageHero: {
    width: 250,
    height: 250,
    resizeMode: 'contain',
  },
  splashImageRow: {
    flexDirection: 'row',
    gap: 30,
    marginTop: -20,
  },
  splashImageSecondary: {
    width: 100,
    height: 100,
    resizeMode: 'contain',
    opacity: 0.6,
  },
  splashButtonContainer: {
    alignItems: 'flex-end',
    marginBottom: 40,
  },
  splashButton: {
    width: 80,
    height: 80,
    borderRadius: 40,
    backgroundColor: '#FFFFFF',
    justifyContent: 'center',
    alignItems: 'center',
    marginBottom: 10,
    shadowColor: '#00D2FF',
    shadowOffset: { width: 0, height: 0 },
    shadowOpacity: 0.5,
    shadowRadius: 10,
    elevation: 5,
  },
  splashButtonText: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: '600',
    marginRight: 10,
  },
  backButton: {
      padding: 10,
      marginRight: 10,
  },
  // --- Menu Screen Styles ---
  menuContainer: {
      flex: 1,
      backgroundColor: '#101024',
      paddingTop: 50,
  },
  menuHeader: {
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      paddingHorizontal: 20,
      marginBottom: 20,
      marginTop: 10,
  },
  menuBackBtn: {
      padding: 10,
  },
  menuTitle: {
      fontSize: 24,
      fontWeight: 'bold',
      color: '#00D2FF',
      marginLeft: 10,
  },
  menuListContent: {
      paddingHorizontal: 20,
      paddingBottom: 40,
  },
  menuListItem: {
      flexDirection: 'row',
      alignItems: 'center',
      backgroundColor: '#1E1E38',
      borderRadius: 15,
      padding: 20,
      marginBottom: 15,
      borderWidth: 1,
      borderColor: '#333',
  },
  menuListIcon: {
      width: 60,
      height: 60,
      justifyContent: 'center',
      alignItems: 'center',
      backgroundColor: '#2A2A4A',
      borderRadius: 30,
      marginRight: 20,
  },
  menuListInfo: {
      flex: 1,
  },
  menuListName: {
      fontSize: 20,
      fontWeight: 'bold',
      color: '#FFFFFF',
      marginBottom: 5,
  },
  menuListPrice: {
      fontSize: 16,
      color: '#00D2FF',
      fontWeight: '600',
  },
  // --- Standard Customization Styles ---
  standardContainer: {
      width: '90%',
      alignItems: 'center',
  },
  standardImageContainer: {
      marginBottom: 30,
  },
  controlGroup: {
      width: '100%',
      marginBottom: 20,
  },
  controlLabel: {
      fontSize: 16,
      fontWeight: 'bold',
      marginBottom: 10,
      color: '#FFFFFF',
  },
  segmentControl: {
      flexDirection: 'row',
      backgroundColor: '#2A2A4A',
      borderRadius: 10,
      padding: 2,
  },
  segmentBtn: {
      flex: 1,
      paddingVertical: 10,
      alignItems: 'center',
      borderRadius: 8,
  },
  segmentBtnActive: {
      backgroundColor: '#00D2FF',
  },
  segmentText: {
      color: '#FFFFFF',
  },
  checkoutButton: {
      backgroundColor: '#00FF88',
      paddingVertical: 15,
      paddingHorizontal: 40,
      borderRadius: 10,
      marginTop: 20,
      width: '100%',
      alignItems: 'center',
  },
  checkoutButtonText: {
      color: '#101024',
      fontSize: 18,
      fontWeight: 'bold',
  },
  headerTitle: {
      fontSize: 20,
      fontWeight: 'bold',
      color: '#FFFFFF',
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: '600',
    marginBottom: 10,
    marginTop: 20,
    color: '#FFFFFF',
  },
  ingredientText: {
    fontWeight: 'bold',
    color: '#333',
  },
  sizeControl: {
      alignItems: 'center',
      width: '100%',
  },
  sizeButtons: {
      flexDirection: 'row',
      gap: 20,
  },
  sizeBtn: {
      width: 50,
      height: 50,
      borderRadius: 25,
      backgroundColor: '#2A2A4A',
      justifyContent: 'center',
      alignItems: 'center',
  },
  activeSizeBtn: {
      backgroundColor: '#00D2FF',
  },
  sizeBtnText: {
      fontSize: 16,
      color: '#FFFFFF',
  },
  activeSizeBtnText: {
      color: '#101024',
  },
  absoluteBackBtn: {
      position: 'absolute',
      top: 50,
      left: 20,
      zIndex: 10,
      padding: 10,
      backgroundColor: 'rgba(0,0,0,0.3)',
      borderRadius: 25,
  },
  menuItemImage: {
      width: 50,
      height: 50,
      resizeMode: 'contain',
  },
  standardImage: {
      width: 200,
      height: 200,
      resizeMode: 'contain',
  },
  orderContainer: {
      flex: 1,
      backgroundColor: '#101024',
      justifyContent: 'center',
      alignItems: 'center',
      padding: 20,
  },
  orderTitle: {
      fontSize: 32,
      fontWeight: 'bold',
      color: '#FFFFFF',
      marginTop: 20,
  },
  orderId: {
      fontSize: 48,
      fontWeight: 'bold',
      color: '#00D2FF',
      marginVertical: 20,
  },
  orderSubtitle: {
      fontSize: 18,
      color: '#CCCCCC',
      marginBottom: 40,
  },
  homeButton: {
      backgroundColor: '#00FF88',
      paddingVertical: 15,
      paddingHorizontal: 40,
      borderRadius: 30,
  },
  pendingOrdersContainer: {
      marginTop: 20,
      paddingHorizontal: 20,
      borderTopWidth: 1,
      borderTopColor: '#333',
      paddingTop: 10,
  },
  pendingOrdersTitle: {
      fontSize: 18,
      fontWeight: 'bold',
      color: '#FFFFFF',
      marginBottom: 10,
  },
  pendingOrderCard: {
      backgroundColor: '#2A2A4A',
      padding: 15,
      borderRadius: 10,
      marginRight: 10,
      width: 120,
  },
  pendingOrderId: {
      color: '#00D2FF',
      fontWeight: 'bold',
      fontSize: 16,
  },
  pendingOrderName: {
      color: '#FFFFFF',
      fontSize: 14,
      marginVertical: 5,
  },
  pendingOrderStatus: {
      color: '#888',
      fontSize: 12,
      fontStyle: 'italic',
  },
});
