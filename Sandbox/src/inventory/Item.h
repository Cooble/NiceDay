#pragma once
#include "ndpch.h"
#include "memory/Pool.h"
#include "core/sids.h"
#include "core/NBT.h"

#define ND_REGISTER_ITEM(item)\
	ItemRegistry::get().registerItem(item);


constexpr int ITEMSTACK_POOL_SIZE = 1000;

class WorldEntity;
class World;
class ItemRegistry;
class TextureAtlas;
class ItemStack;
typedef uint64_t ItemID;

class Block;
struct BlockStruct;


constexpr int ITEM_FLAG_IS_BLOCK = 0;
constexpr int ITEM_FLAG_HAS_NBT = 1;

// if the texture metadata is determined by metadata
constexpr int ITEM_FLAG_USE_META_AS_TEXTURE = 2;

constexpr int ITEM_FLAG_ARMOR_HEAD = 3;
constexpr int ITEM_FLAG_ARMOR_CHEST = 4;
constexpr int ITEM_FLAG_ARMOR_LEGGINS = 5;
constexpr int ITEM_FLAG_ARMOR_BOOTS = 6;
constexpr int ITEM_FLAG_AMMO = 7;





class Item
{
protected:
  const ItemID m_id;
  int m_max_stack_size;
  const std::string m_text_name;
  half_int m_texture_pos;
  int m_flags = 0;

  // how many types of the same exists (tells creative tab to list all possibilities)
  int m_max_metadata = 0;

  constexpr void setFlag(int flag, bool value = true) {
	if (value)
	  m_flags |= (1 << flag);
	else
	  m_flags &= ~(1 << flag);
  }
public:

  enum Interaction
  {
	PRESSED, RELEASED
  };
  inline const static int INFINITE_SIZE = -1;
  virtual ~Item() = default;
  Item(ItemID id, const std::string& textName);


  Item& setMaxStackSize(int size) { m_max_stack_size = size; return *this; }

  int getMaxMeta()const { return m_max_metadata; }

  virtual void onTextureLoaded(const TextureAtlas& atlas);
  virtual int getTextureOffset(const ItemStack& b) const;

  int getMaxStackSize() const { return m_max_stack_size; }
  ItemID getID() const { return m_id; }
  const std::string& toString() const { return m_text_name; }

  // flags
  constexpr bool hasFlag(int flag) const {
	return m_flags & (1 << flag);
  }
  constexpr int isUseMetaAsTexture() const { return hasFlag(ITEM_FLAG_USE_META_AS_TEXTURE); }
  constexpr int hasNBT() const { return hasFlag(ITEM_FLAG_HAS_NBT); }
  constexpr int isBlock() const { return hasFlag(ITEM_FLAG_IS_BLOCK); }

  virtual int getBlockID() const;



  //=============================EVENTS=============================

  // called when entity chooses this item to be held in hand
  virtual void onEquipped(World& world, ItemStack& stack, WorldEntity& owner) const;

  // called when entity switches from this item to be held in hand
  virtual void onUnequipped(World& world, ItemStack& stack, WorldEntity& owner) const;

  // called when entity wants to use this item
  virtual bool onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const { return false; }

  // called when player uses item on block
  // return true if event was consumed
  virtual bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const { return onRightClick(world, stack, owner, x, y); }

  // called when player uses item on entity
  virtual bool onRightClickOnEntity(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, WorldEntity& target) const { return onRightClick(world, stack, owner, x, y); }

  // called before the item is thrown away
  virtual void onDisposed(World& world, ItemStack& stack, WorldEntity& owner) const {}

  // return true if entity was damaged
  virtual bool hitEntity(World& w, ItemStack* stack, WorldEntity& owner, WorldEntity& target)const { return false; }

  // called each tick the item is used to dig some block
  virtual void onBlockBeingDigged(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const {}

  // Called each tick when item is held in hand = active slot
  virtual void onItemHeldInHand(World& w, WorldEntity& owner, ItemStack& stack) const {}

  // returns pointer to new custom structure that will hold temporary data of item or nullptr
  // is called before onEquipped
  virtual void* instantiateDataBox() const { return nullptr; }

  // destroys created dataBox returned by instantiateDataBox()
  virtual void destroyDataBox(void* dataBox) const {}

  // called when clicked with held item somewhere at world location x,y
  // Params:
  //		ticksPressed number of ticks this item is pressed. When clicked first ticksPressed is 0
  //		stack held item, if stack.size() is zero stack will be automatically destroyed. -> don't destroy the item!
  //		dataBox custom data structure pointer returned by instantiateDataBox() call (will be reset each onEquipped())
  virtual void onInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const {}

  virtual std::string getTitle(ItemStack* stack)const;
};


class ItemRegistry
{
private:
  std::unordered_map<ItemID, Item*> m_items;
  ItemRegistry() = default;
public:
  ItemRegistry(ItemRegistry const&) = delete;
  void operator=(ItemRegistry const&) = delete;
  ~ItemRegistry();

  inline const auto& getItems() { return m_items; }

  void initTextures(const TextureAtlas& atlas);

  static inline ItemRegistry& get() {
  static ItemRegistry s_instance;
	return s_instance;
  }
public:
  //takes ownership
  void registerItem(Item* item);
  const Item& getItem(ItemID id) const;
};

class ItemStack
{
private:
  static Pool<ItemStack>& s_stack_pool();
public:
  static ItemStack* create(ItemID id, int  count = 1);
  static ItemStack* create(const ItemStack* itemstack);
  static ItemStack* deserialize(const NBT& nbt);
  static void destroy(ItemStack* stack);
private:
  ItemID m_item;
  uint64_t m_metadata = 0;
  int m_size;
  NBT m_nbt;
public:
  ItemStack(ItemID item, int size = 1);
  ItemStack(const ItemStack& s);
  ~ItemStack();

  uint64_t getMetadata() const { return m_metadata; }
  void setMetadata(uint64_t meta) { m_metadata = meta; }
  void setSize(int size) { m_size = size; }
  int size()const { return m_size; }
  bool isEmpty() const { return m_size == 0; }
  ItemID getItemID() const { return m_item; }
  const Item& getItem() const { return ItemRegistry::get().getItem(m_item); }
  const NBT& getNBT() const { return m_nbt; }
  NBT& getNBT() { return m_nbt; }
  void destroy() { ItemStack::destroy(this); }
  ItemStack* copy() const { return ItemStack::create(this); }
  bool equals(const ItemStack* stack) const;

  void addSize(int count)
  {
	if (m_size == Item::INFINITE_SIZE)//ignore change if item has infinite size
	  return;
	ASSERT(m_size + count <= getItem().getMaxStackSize(), "Too big itemstack, ({}/{})", m_size + count, getItem().getMaxStackSize());
	m_size += count;
	if (m_size < 0)
	  m_size = 0;
  }
  void serialize(NBT& nbt);
  bool isFullStack() const;
};
bool operator==(const ItemStack& a, const ItemStack& b);

